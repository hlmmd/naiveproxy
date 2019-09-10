#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> //inet_ntoa()
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/epoll.h>

#include "proxy.hpp"
#include "naiveconfig.hpp"
#include "naiveproxy.hpp"

httpproxy::~httpproxy()
{
    clientfds.clear();
    if (events)
    {
        delete[] events;
        events = NULL;
    }
    if (cfg)
    {
        delete[] cfg;
        cfg = NULL;
    }
}

int httpproxy::do_accept(int connectfd)
{
    int dest_fd = 0;
    dest_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(cfg->dest_port);
    dest_addr.sin_addr.s_addr = cfg->dest_ipaddr;

    setnonblocking(dest_fd);
    setnonblocking(connectfd);
    connect(dest_fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (errno != EINPROGRESS)
    {
        //perror("connect");
        return -1;
    }

    sfh[connectfd] = dest_fd;
    sfh[dest_fd] = connectfd;

    clientfds.insert(connectfd);

    //连接计数+1
    cfg->client_num++;

    //将两个socket添加到epoll事件中
    epoll_addfd(connectfd);
    epoll_addfd(dest_fd);

    return dest_fd;
}

int httpproxy::handle_http_header(unsigned char *buf, int len)
{
    int ret = len;
    //printf("%s\n", buf);
    unsigned char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);
    const char Hoststr[] = "Host:";
    const char linestr[] = "\r\n";
    char *p = strstr((char *)buf, Hoststr);
    if (p == NULL)
        return ret;
    p += sizeof(Hoststr);

    int start = p - (char *)buf;

    p = strstr(p, linestr);
    if (p == NULL)
        return ret;

    //\r\n后面的所有Http头
    int end = (p - (char *)buf);
    memcpy(buffer, p, len - end);

    for (int i = 0; cfg->hostname[i]; i++)
    {
        buf[i + start] = cfg->hostname[i];
    }

    memcpy(buf + start + cfg->hostname.length(), buffer, len - end);

    ret = len + end - start + cfg->hostname.length();

    //printf("%s\n", buf);

    return ret;
}

int httpproxy::do_read(int sockfd, unsigned char *buf, int len)
{
    //查看是不是client端
    bool isclient = (clientfds.find(sockfd) != clientfds.end());

    int destfd = sfh[sockfd];
    long long total = 0;
    while (1)
    {
        memset(buf, 0, MAX_BUFFER_SIZE);
        int ret = recv(sockfd, buf, MAX_BUFFER_SIZE, 0);

        if (ret > 0)
        {
            total += ret;

            //处理client的http请求
            if (isclient && cfg->hostname.length() != 0)
            {
                ret = handle_http_header(buf, ret);
            }

            do_send(destfd, buf, ret);
        }
        //这次的EPOLLIN事件已经读完了
        else if (ret < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            break;
        }
        //断开连接
        else
        {
            cfg->client_num--;
            epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, events);
            epoll_ctl(epollfd, EPOLL_CTL_DEL, destfd, events);
            close(sockfd);
            close(destfd);

            if (isclient)
                clientfds.erase(sockfd);
            else
                clientfds.erase(destfd);

            sfh.erase(sockfd);
            sfh.erase(destfd);
            //printf("curren: %d\n", cfg->client_num);
            break;
        }
    }
    return total;
}
