
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

int nproxy_tcp_sendpeer(struct nproxy *client, int peer_fd, char *buffer, int length)
{

    char logstr[1024];
    memset(logstr, 0, sizeof(logstr));
    int sendcount = 0;
    struct timeval tv;
    tv.tv_sec = 1;
    while (sendcount < length)
    {
        fd_set wrfs;
        FD_ZERO(&wrfs);
        FD_SET(peer_fd, &wrfs);
        int ret = 0;

        //ret = select(peer_fd + 1, NULL, &wrfs, NULL, NULL);
        ret = select(peer_fd + 1, NULL, &wrfs, NULL, &tv);

        if (ret < 0)
            return sendcount;
        else if (ret == 0)
            return 0;
        else
        {
            sendcount += send(peer_fd, buffer + sendcount, length - sendcount, 0);
        }
    }

    return sendcount;
}

int tcp_fork_connection(struct nproxy *client, int logfd)
{
    struct nproxy_config *cfg = (struct nproxy_config *)client;

    //print_one_config(cfg);
    struct in_addr a;
    a.s_addr = client->dest_ipaddr;
    char *p = inet_ntoa(a);

    char logstr[1024];

    int dest_fd = setup_tcp_dest_connection(client);
    client->dest_fd = dest_fd;
    client->logfd = logfd;
    fd_set rdfs;
    int ret;
    int maxfd = 0;
    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        //maxfd = max(client->client_fd, client->dest_fd);
        maxfd = client->client_fd > client->dest_fd ? client->client_fd : client->dest_fd;
        FD_ZERO(&rdfs);
        FD_SET(client->client_fd, &rdfs);
        FD_SET(client->dest_fd, &rdfs);
        int ret = select(maxfd + 1, &rdfs, NULL, NULL, NULL);
        if (ret < 0)
            return 0;
        else if (ret == 0)
            continue;
        else
        {
            if (FD_ISSET(client->client_fd, &rdfs))
            {
                while (1)
                {
                    int ret = recv(client->client_fd, buffer, MAX_BUFFER_SIZE, 0);
                    if (ret > 0)
                    {
                        int writebyte = nproxy_tcp_sendpeer(client, client->dest_fd, buffer, ret);
                        if (writebyte <= 0)
                        {
                            close(client->dest_fd);
                            exit(0);
                        }
                        if (ret < MAX_BUFFER_SIZE)
                            break;
                    }
                    else
                    {
                        close(client->client_fd);
                        //断开连接
                        exit(0);
                    }
                }
            }
            if (FD_ISSET(client->dest_fd, &rdfs))
            {
                while (1)
                {
                    int ret = recv(client->dest_fd, buffer, MAX_BUFFER_SIZE, 0);
                    if (ret > 0)
                    {
                        //nproxy_log(logfd, buffer, ret);
                        int writebyte = nproxy_tcp_sendpeer(client, client->client_fd, buffer, ret);
                        if (writebyte <= 0)
                        {
                            close(client->client_fd);
                            exit(0);
                        }
                        if (ret < MAX_BUFFER_SIZE)
                            break;
                    }
                    else
                    {
                        close(client->dest_fd);
                        //断开连接
                        exit(0);
                    }
                }
            }
        }
    }

    //    printf("my pid:%d   ppid:%d\n", getpid(), getppid());
    return 0;
}

int start_udp_nproxy(struct nproxy_config *cfg)
{
    return 0;
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

    while (1)
    {
        int ret = 0;
        fd_set rfds;
        int maxfd = listen_sockfd;
        //清空集合
        FD_ZERO(&rfds);
        //将当前连接和listen socket句柄加入到集合中
        FD_SET(listen_sockfd, &rfds);

        ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1)
        {
            printf("select error\n");
            break;
        }
        else if (ret == 0)
            continue;
        else
        {
            if (FD_ISSET(listen_sockfd, &rfds))
            {
                struct sockaddr_in client_addr;
                socklen_t client_addr_length = sizeof(client_addr);
                int clientfd;

                if ((clientfd = accept(listen_sockfd, (struct sockaddr *)&client_addr, &client_addr_length)) > 0)
                {
                    struct nproxy *client = (struct nproxy *)malloc(sizeof(struct nproxy));
                    memset(client, 0, sizeof(struct nproxy));
                    memcpy(client, cfg, sizeof(struct nproxy_config));
                    client->client_fd = clientfd;

                    char logstr[1024];
                    int ret = sprintf(logstr, "Connected from %s,type:%s\n", inet_ntoa(client_addr.sin_addr),
                                      cfg->io_type == IO_TYPE_INOUT ? "INOUT" : "OUTIN");

                    nproxy_log(logfd, logstr, ret);
                    //忽略SIGCHLD，避免僵尸进程
                    signal(SIGCHLD, SIG_IGN);

                    pid_t pid;
                    if ((pid = fork()) < 0)
                    {
                        printf("fork error\n");
                        exit(-1);
                    }
                    else if (pid == 0)
                    {
                        free(cfg);
                        cfg = NULL;
                        close(listen_sockfd);

                        tcp_fork_connection(client, logfd);

                        exit(0);
                    }
                    close(clientfd);
                    free(client);
                    client = NULL;
                }
            }
        }
    }

    close(listen_sockfd);
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