
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include <assert.h>
#include <sys/epoll.h>
#include "nproxy.h"
#include "nproxy-config.h"
#include "nproxy-log.h"

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int setup_tcp_dest_connection(struct nproxy *client)
{
    int dest_fd;
    dest_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(client->dest_port);
    dest_addr.sin_addr.s_addr = client->dest_ipaddr;

    setnonblocking(dest_fd);

    connect(dest_fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (errno != EINPROGRESS)
    {
        perror("connect");
        exit(errno);
    }
    client->dest_fd = dest_fd;
    //printf("connected server !\n");

    return dest_fd;
}

int start_udp_nproxy(struct nproxy_config *cfg)
{
    return 0;
}

void tcp_et(struct epoll_event *events, int number, int epollfd, int listenfd)
{
//     char buf[MAX_BUFFER_SIZE];
//     for (int i = 0; i < number; i++)
//     {
//         int sockfd = events[i].data.fd;
//         if (sockfd == listenfd)
//         {
//             struct sockaddr_in client_address;
//             socklen_t client_addrlength = sizeof(client_address);

//             while (1)
//             {
//                 if (connected >= USER_LIMIT)
//                     continue;
//                 int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
//                 //   if (connfd < 0 && (errno == EAGAIN || errno == ECONNABORTED || errno == EPROTO || errno == EINTR))
//                 if (connfd < 0 && (errno == EAGAIN))
//                     break;
//                 else if (connfd < 0)
//                 {
//                     printf("<0\n");
//                 }

//                 addfd(epollfd, connfd, true);
//                 printf("current user:%d\n", connected);
//             }
//         }
//         else if (events[i].events & EPOLLIN)
//         {
//             while (1)
//             {
//                 memset(buf, '\0', BUFFER_SIZE);
//                 int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
//                 if (ret < 0)
//                 {
//                     if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
//                     {
//                         //     printf("read later\n");
//                         break;
//                     }
//                     close(sockfd);
//                     break;
//                 }
//                 else if (ret == 0)
//                 {
//                     epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, events);
//                     close(sockfd);
//                     connected--;
//                     printf("current user:%d\n", connected);
//                     break;
//                 }
//                 else
//                 {
//                     //      printf("get %d bytes of content: %s\n", ret, buf);
//                 }
//             }
//         }
//         else
//         {
//             printf("something else happened \n");
//         }
//     }
}

int start_tcp_nproxy(struct nproxy_config *cfg)
{
    int logfd = init_log();

    int listen_sockfd;
    int ret;
    struct sockaddr_in nproxy_addr;

    nproxy_addr.sin_family = AF_INET;
    nproxy_addr.sin_port = htons(cfg->proxy_port);
    nproxy_addr.sin_addr.s_addr = cfg->proxy_ipaddr;

    listen_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(listen_sockfd > 0);

    if (1)
    {
        int opt = 1;
        setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    ret = bind(listen_sockfd, (struct sockaddr *)&nproxy_addr, sizeof(nproxy_addr));
    assert(ret != -1);

    setnonblocking(listen_sockfd);

    ret = listen(listen_sockfd, 10);
    assert(ret != -1);

    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * USER_LIMIT);

    //epoll_create的第二个参数已经被忽略了。
    int epollfd = epoll_create(USER_LIMIT);
    assert(epollfd != -1);
    // addfd(epollfd, listen_sockfd, true);

    // // connected--;
    // while (1)
    // {
    //     int ret = epoll_wait(epollfd, events, USER_LIMIT, -1);
    //     if (ret < 0)
    //     {
    //         printf("epoll failure\n");
    //         break;
    //     }
    //     tcp_et(events, ret, epollfd, listen_sockfd);
    // }
    // close(listen_sockfd);

    free(events);
    return 0;
}

int start_nproxys()
{

    int logfd = init_log();
    signal(SIGCHLD, SIG_IGN);

    struct nproxy_config *cfgs = NULL;
    //初始化config，读取config文件，返回读到的config的个数
    int nproxy_config_size = init_nproxy_config(&cfgs);

    //申请一个config结构，用于读取一个cfg
    struct nproxy_config *cfg = (struct nproxy_config *)malloc(sizeof(struct nproxy_config));
    if (cfg == NULL)
        exit(0);
    for (int i = 0; i < nproxy_config_size; i++)
    {
        //使用memcpy从cfgs中复制一份config，因为要fork，在子进程中会将cfgs释放，所以重新复制了一份，子进程只需要一个cfg
        memcpy(cfg, cfgs + i, sizeof(struct nproxy_config));
        pid_t pid;
        pid = fork();
        if (pid == 0)
        {
            destroy_log(logfd);
            //释放父进程打开的文件描述符和申请的cfgs空间。
            for (i = 0; i < NOFILE; i++)
                close(i);
            destroy_nproxy_config(&cfgs);

            //根据不同的protocol调用不同的API
            if (cfg->protocol == PROTOCOL_TCP)
                start_tcp_nproxy(cfg);
            else
                start_udp_nproxy(cfg);
            exit(0);
        }
        else if (pid < 0)
        {
            exit(0);
        }
    }

    //释放cfg和cfgs
    free(cfg);
    cfg = NULL;
    destroy_nproxy_config(&cfgs);

    return 0;
}