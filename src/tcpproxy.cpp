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

tcpproxy::tcpproxy(naiveconfig *cfg1)
{
    events = NULL;
    cfg = cfg1;
}

tcpproxy::~tcpproxy()
{
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

int tcpproxy::setnonblocking(uint32 fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void tcpproxy::epoll_addfd(int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

int tcpproxy::do_accept(int connectfd)
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

    //连接计数+1
    cfg->client_num++;

    //将两个socket添加到epoll事件中
    epoll_addfd(connectfd);
    epoll_addfd(dest_fd);

    return dest_fd;
}

int tcpproxy::do_send(int sockfd, unsigned char *buf, int len)
{
    int sended = 0;
    while (sended < len)
    {
        int sendret = send(sockfd, buf + sended, len - sended, 0);
        if (sendret < 0 && errno == EAGAIN)
        {
            continue;
        }
        else if (sendret <= 0)
        {
            break;
        }
        sended += sendret;
    }
    return sended;
}

int tcpproxy::do_read(int sockfd, unsigned char *buf, int len)
{
    int destfd = sfh[sockfd];
    long long total = 0;
    while (1)
    {
        memset(buf, 0, MAX_BUFFER_SIZE);
        int ret = recv(sockfd, buf, MAX_BUFFER_SIZE, 0);
        if (ret > 0)
        {
            total += ret;
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
            sfh.erase(sockfd);
            sfh.erase(destfd);
            //printf("curren: %d\n", cfg->client_num);
            break;
        }
    }
    return total;
}

void tcpproxy::handle_events(int number)
{
    int listenfd = cfg->listen_sockfd;
    unsigned char buf[MAX_BUFFER_SIZE];
    for (int i = 0; i < number; i++)
    {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd)
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);

            //epoll的et模式下面，accept需要用while循环
            while (1)
            {
                if (cfg->client_num >= USER_LIMIT)
                    continue;
                int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                if (connfd < 0 && (errno == EAGAIN))
                    break;
                else if (connfd < 0)
                {
                    return;
                }
                // char logstr[1024];
                // int ret = sprintf(logstr, "Connected from %s\n", inet_ntoa(client_address.sin_addr));

                // naivelog::GetInstance()->write_log(logstr, ret);
                //  nproxy_log(cfg->logfd, logstr, ret);

                do_accept(connfd);

                //printf("curren: %d\n", cfg->client_num);

                //这里，在connect dest后，由于是非阻塞，此时tcp连接可能还未建立，如果直接send，会返回-1
                // sleep(1); //等待tcp连接建立
            }
        }
        //只处理EPOLLIN事件。触发之后直接写map对应的socket，不考虑缓冲区满的情况。
        else if (events[i].events & EPOLLIN)
        {
            if (sfh.find(sockfd) != sfh.end())
            {
                do_read(sockfd, buf, 0);
                //如果对应的哈希表项已经被free了，说明该连接已经释放
                //continue;
            }
        }
    }
}

void tcpproxy::startproxy()
{
    if (naiveproxy::GetInstance()->Isdaemonized()==false)
    {
        printf("start %s proxy\n", protocol_name[cfg->protocol]);
        cfg->print();
    }

    int ret;
    struct sockaddr_in nproxy_addr;

    nproxy_addr.sin_family = AF_INET;
    nproxy_addr.sin_port = htons(cfg->proxy_port);
    nproxy_addr.sin_addr.s_addr = cfg->proxy_ipaddr;

    cfg->listen_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(cfg->listen_sockfd > 0);

    if (1)
    {
        int opt = 1;
        setsockopt(cfg->listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    setnonblocking(cfg->listen_sockfd);

    ret = bind(cfg->listen_sockfd, (struct sockaddr *)&nproxy_addr, sizeof(nproxy_addr));
    if (ret != 0)
    {
        return;
    }

    ret = listen(cfg->listen_sockfd, 10);
    assert(ret != -1);
    if (events == NULL)
        events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * USER_LIMIT);
    if (events == NULL)
        exit(0);

    //epoll_create的第二个参数已经被忽略了。
    epollfd = epoll_create(USER_LIMIT);
    assert(epollfd != -1);
    epoll_addfd(cfg->listen_sockfd);

    while (1)
    {
        int ret = epoll_wait(epollfd, events, USER_LIMIT, -1);
        if (ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
        handle_events(ret);
        // tcp_et_events(events, ret, epollfd, cfg);
    }

    // destroy_hashmap(&cfg->sfh);
    close(cfg->listen_sockfd);
    // free(cfg);
    // free(events);

    return;
}
