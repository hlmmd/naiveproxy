

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


int forkfunc()
{
    printf("my pid:%d   ppid:%d\n",getpid(),getppid());
    return 0;
}


int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int start_tcp_nproxy(short io_type,
                     int proxy_ipaddr, //proxy server ip
                     short proxy_port, //proxy service port
                     int dest_ipaddr,
                     short dest_port)
{

    int listen_sockfd;
    int ret;
    struct sockaddr_in nproxy_addr;

    nproxy_addr.sin_family = AF_INET;
    nproxy_addr.sin_port = htons(proxy_port);
    nproxy_addr.sin_addr.s_addr = proxy_ipaddr;

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
                    printf("Connected from %s\n", inet_ntoa(client_addr.sin_addr));
                    //printf("accept succ\n");
                    //signal(SIGCHLD, SIG_IGN);

                    pid_t pid;
                    if ((pid = fork()) < 0)
                    {
                        printf("fork error\n");
                        exit(-1);
                    }
                    else if (pid == 0)
                    {
                        close(listen_sockfd);
                        forkfunc();                        
                        exit(0);
                    }
                    close(clientfd);
                }
            }
        }
    }

    close(listen_sockfd);
    return 0;
}

int start_nproxys()
{

    struct nproxy_config *cfg;

    start_tcp_nproxy(0, 0, 6000, 0, 0);
    //start_tcp_nproxy(io_type,proxy_ipaddr,proxy_port,dest_ipaddrdest_port);

    return 0;
}