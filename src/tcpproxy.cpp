#include "proxy.hpp"

#include <cstdlib>

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h> //inet_ntoa()
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/param.h>
#include <sys/select.h>

#include <vector>
#include <string>
#include "naiveconfig.hpp"
#include "naiveproxy.hpp"
#include <iostream>
#include <assert.h>
#include <sys/epoll.h>

tcpproxy::tcpproxy(naiveconfig *cfg1)
{
    cfg = cfg1;
}

tcpproxy::~tcpproxy()
{
    delete cfg;
    cfg = NULL;
}

int tcpproxy::setnonblocking(uint32 fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void tcpproxy::epoll_addfd(int epollfd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void tcpproxy::startproxy()
{
    std::cout << "start tcp proxy" << std::endl;
    cfg->print();

    cfg->logfd = naiveproxy::init_logfd();

    //初始化cfg的哈希表
    //init_hashmap(&cfg->sfh);

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

    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * USER_LIMIT);

    //epoll_create的第二个参数已经被忽略了。
    int epollfd = epoll_create(USER_LIMIT);
    assert(epollfd != -1);
    epoll_addfd(epollfd, cfg->listen_sockfd);

    while (1)
    {
        int ret = epoll_wait(epollfd, events, USER_LIMIT, -1);
        if (ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
        //  tcp_et_events(events, ret, epollfd, cfg);
    }

    // destroy_hashmap(&cfg->sfh);
    close(cfg->listen_sockfd);
    close(cfg->logfd);
    // free(cfg);
    // free(events);

    return;
}
