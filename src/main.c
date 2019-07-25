
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
#include <assert.h>
#include <sys/epoll.h>
#include <sys/stat.h>

#include "nproxy.h"

int main()
{

  
    //设置为守护进程
    daemonize();

    //保证只有一个主进程实例
    int oncefd = open_only_once();

    start_nproxys();

    while (1)
        ;

    close(oncefd);
    return 0;
}