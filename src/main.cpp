
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

#include "naiveproxy.hpp"

int main()
{  

    naiveproxy* np = naiveproxy::GetInstance();

    //np->daemonize();

    np->open_only_once();

    while(1)
        ;
    
    naiveproxy::DestroyInstance();

    return 0;

  //  start_nproxys();

    return 0;
}