
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>
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

#ifndef NDEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <sys/epoll.h>
#include "nproxy.h"
#include "nproxy-config.h"
#include "nproxy-log.h"

int open_only_once()
{
    const char filename[] = "/tmp/naiveproxy.pid";
    int fd, val;
    char buf[10];
    //打开控制文件，控制文件打开方式：O_WRONLY | O_CREAT只写创建方式
    //控制文件权限：S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH用户、用户组读写权限
    if ((fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
        return -1;
    }
    // try and set a write lock on the entire file
    struct flock lock;
    //建立一个供写入用的锁
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    //以文件开头为锁定的起始位置
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    //结合lock中设置的锁类型，控制文件设置文件锁，此处设置写文件锁
    if (fcntl(fd, F_SETLK, &lock) < 0)
    {
        //如果获取写文件锁成功，则退出当前进程，保留后台进程
        if (errno == EACCES || errno == EAGAIN)
        {
            //   printf("naiveproxy has already run.\n");
            exit(-1); // gracefully exit, daemon is already running
        }
        else
        {
            //   printf("file being used\n");
            return -1; //如果锁被其他进程占用，返回 -1
        }
    }
    // truncate to zero length, now that we have the lock
    //改变文件大小为0
    if (ftruncate(fd, 0) < 0)
        return -1;
    // and write our process ID
    //获取当前进程pid
    sprintf(buf, "%d\n", getpid());
    //将启动成功的进程pid写入控制文件
    if (write(fd, buf, strlen(buf)) != strlen(buf))
        return -1;

    // set close-on-exec flag for descriptor
    // 获取当前文件描述符close-on-exec标记
    if ((val = fcntl(fd, F_GETFD, 0)) < 0)
        return -1;
    val |= FD_CLOEXEC;
    //关闭进程无用文件描述符
    if (fcntl(fd, F_SETFD, val) < 0)
        return -1;
    // leave file open until we terminate: lock will be held
    return fd;
}

int daemonize()
{
    int pid;
    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
        exit(1);
    //创建会话期
    setsid();

    //fork 两次
    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
        exit(1);

    //设置工作目录，设置为/tmp保证具有权限
    int ret = chdir("/tmp");
    if (ret != 0)
    {
        printf("chdir失败\n");
        return -1;
    }

    //设置权限掩码
    umask(0);

    //关闭已经打开的文件描述符
    //for (int i = 0; i < getdtablesize(); i++)
    for (int i = 0; i < 2; i++)
        close(i);

    //忽略SIGCHLD信号，防止产生僵尸进程
    signal(SIGCHLD, SIG_IGN);
    return 1;
}

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void epoll_addfd(int epollfd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

int setup_tcp_dest_connection(int epollfd, int connectfd, struct nproxy_config *cfg)
{
    int dest_fd;
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
    //建立socket的哈希映射
    if (find_hashmap(cfg->sfh, connectfd) == 0)
    {
        insert_hashmap(cfg->sfh, connectfd, dest_fd);
    }
    else
    {
        update_hashmap(cfg->sfh, connectfd, dest_fd);
    }

    if (find_hashmap(cfg->sfh, dest_fd) == 0)
    {
        insert_hashmap(cfg->sfh, dest_fd, connectfd);
    }
    else
    {
        update_hashmap(cfg->sfh, dest_fd, connectfd);
    }

    //连接计数+1
    cfg->client_num++;

    //将两个socket添加到epoll事件中
    epoll_addfd(epollfd, connectfd);
    epoll_addfd(epollfd, dest_fd);

    return dest_fd;
}

int start_udp_nproxy(struct nproxy_config *cfg)
{
    return 0;
}

int do_accept(int epollfd, int connectfd, struct nproxy_config *cfg)
{

    int ret = setup_tcp_dest_connection(epollfd, connectfd, cfg);
    //未能建立到dest的连接返回-1
    return ret;
}

void tcp_et_events(struct epoll_event *events, int number, int epollfd, struct nproxy_config *cfg)
{
    int listenfd = cfg->listen_sockfd;
    char buf[MAX_BUFFER_SIZE];
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
                    //printf("<0\n");
                    return 0;
                }

                char logstr[1024];

                int ret = sprintf(logstr, "Connected from %s\n", inet_ntoa(client_address.sin_addr));

                nproxy_log(cfg->logfd, logstr, ret);

                do_accept(epollfd, connfd, cfg);

                //printf("curren: %d\n", cfg->client_num);

                //这里，在connect dest后，由于是非阻塞，此时tcp连接可能还未建立，如果直接send，会返回-1
                // sleep(1); //等待tcp连接建立
            }
        }
        //只处理EPOLLIN事件。触发之后直接写map对应的socket，不考虑缓冲区满的情况。
        else if (events[i].events & EPOLLIN)
        {
            while (1)
            {
                memset(buf, 0, MAX_BUFFER_SIZE);
                int ret = recv(sockfd, buf, MAX_BUFFER_SIZE, 0);
                if (ret > 0)
                {
                    if (find_hashmap(cfg->sfh, sockfd) == 0)
                    {
                        //如果对应的哈希表项已经被free了，说明该连接已经释放
                        break;
                    }
                    int destfd = getvalue_hashmap(cfg->sfh, sockfd);

                    int sended = 0;
                    while (sended < ret)
                    {
                        int sendret = send(destfd, buf + sended, ret - sended, 0);
                        if (sendret < 0 && errno == EAGAIN)
                        {
                            //在connect dest后，由于是非阻塞，此时tcp连接可能还未建立，如果直接send，会返回-1
                            //这里如果send缓冲区满了，就会循环等待。
                            continue;
                        }
                        else if (sendret <= 0)
                        {
                            //   printf("server down\n");
                            break;
                        }
                        sended += sendret;
                    }
                }
                //这次的EPOLLIN事件已经读完了
                else if (ret < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                {
                    break;
                }
                //断开连接
                else
                {
                    if (find_hashmap(cfg->sfh, sockfd) == 0)
                    {
                        //如果对应的哈希表项已经被free了，说明该连接已经释放
                        break;
                    }

                    int destfd = getvalue_hashmap(cfg->sfh, sockfd);

                    cfg->client_num--;
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, events);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, destfd, events);
                    close(sockfd);
                    close(destfd);
                    delete_hashmap(cfg->sfh, sockfd);
                    delete_hashmap(cfg->sfh, destfd);
                    //printf("curren: %d\n", cfg->client_num);
                    break;
                }
            }
        }
    }
}

int start_tcp_nproxy(struct nproxy_config *cfg)
{
    cfg->logfd = init_log();

    //初始化cfg的哈希表
    init_hashmap(&cfg->sfh);

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
        return 0;
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
        tcp_et_events(events, ret, epollfd, cfg);
    }

    destroy_hashmap(&cfg->sfh);
    close(cfg->listen_sockfd);
    destroy_log(cfg->logfd);
    free(cfg);
    free(events);
    return 0;
}

int start_nproxys()
{
    int logfd = init_log();

    int start_count = 0;

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
#if 1
        pid_t pid;
        pid = fork();
        if (pid == 0)
        {
            destroy_log(logfd);
            //释放父进程打开的文件描述符和申请的cfgs空间。
            for (i = 0; i < NOFILE; i++)
                close(i);
            //释放cfgs空间
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

#else
        destroy_log(logfd);
        start_tcp_nproxy(cfg);
#endif
    }

    //释放cfg和cfgs
    free(cfg);
    cfg = NULL;
    destroy_nproxy_config(&cfgs);

    return 0;
}