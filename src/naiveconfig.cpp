#include "naiveproxy.hpp"

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

#include <iostream>

#define USE_O_APPEND
#define LOG_FILE_NAME "/tmp/naiveproxy.log"
#define CONFIG_FILE_NAME "/etc/naiveproxy.conf"

void naiveconfig::remove_comment(char *str)
{
    char *p = str;
    while (*p && *p != '#')
        p++;
    *p = 0;
}

naiveconfig::naiveconfig(char *str)
{
 //   printf("%s\n", str);
    std::vector<std::string> res;
    const char *d = " \t\n";

    //protocol
    char *p_str = strtok(str, d);
    //转成大写
    if (1)
    {
        char *p = p_str;
        while (*p)
        {
            if (*p >= 'a' && *p <= 'z')
                *p -= 'a' - 'A';
            p++;
        }
    }
    if (strcmp(p_str, "TCP") == 0)
        protocol = PROTOCOL_TCP;
    else if (strcmp(p_str, "UDP") == 0)
        protocol = PROTOCOL_UDP;
    else
    {
        printf("config error. support protocol: TCP/UDP");
        return ;
    }

    //proxy_server_ip
    p_str = strtok(NULL, d);
    struct in_addr pro_addr;
    if (inet_aton(p_str, &pro_addr) == 0)
    {
        perror(p_str);
        exit(errno);
    }
    proxy_ipaddr = pro_addr.s_addr;

    //proxy_server_port
    p_str = strtok(NULL, d);
    proxy_port = atoi(p_str);

    // //dest_server_ip
    p_str = strtok(NULL, d);
    if (inet_aton(p_str, &pro_addr) == 0)
    {
        perror(p_str);
        exit(errno);
    }
    dest_ipaddr = pro_addr.s_addr;

    //dest_server_port
    p_str = strtok(NULL, d);
    dest_port = atoi(p_str);
}

void naiveconfig::print()
{
    struct in_addr nproxy_server;
    nproxy_server.s_addr = proxy_ipaddr;

    struct in_addr dest_server;
    dest_server.s_addr = dest_ipaddr;
    char dest_ip[128];
    strcpy(dest_ip, inet_ntoa(dest_server));

    //注意，inet_ntoa是不可重入的。
    printf("PROTOCOL:          %s\n"
           "nproxy_server_ip:  %s\n"
           "nproxy_port:       %d\n"
           "dest_ipaddr:       %s\n"
           "dest_port:         %d\n",
           protocol == PROTOCOL_TCP ? "TCP" : "UDP",
           inet_ntoa(nproxy_server),
           proxy_port,
           dest_ip,
           dest_port);
}