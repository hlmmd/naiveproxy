#ifndef INCLUDE_NPROXY
#define INCLUDE_NPROXY

#define CONFIG_FILE_NAME "/etc/naiveproxy.conf"

#include "nproxy-config.h"
#include "hashmap.h"
//设置4字节对齐
#pragma pack(4)

struct nproxy
{
    //proxy服务IP
    uint32 proxy_ipaddr;
    //目的ip地址
    uint32 dest_ipaddr;
    //proxy服务端口
    uint16 proxy_port;
    //目的端口
    uint16 dest_port;
    //IO类型 IN->OUT/OUT->IN
    uint8 io_type;
    //协议类型 TCP or UDP
    uint8 protocol;
    //填充字节
    uint16 padding;
    //用户IP地址
    uint32 client_ipaddr;

    //client到nproxy的连接的socket
    uint32 client_fd;

    //nproxy到dest的连接的socket
    uint32 dest_fd;

    //logfd
    uint32 logfd;

    //子进程pid
    uint32 child_pid;
    //用户port
    uint16 client_port;
    //状态
    uint8 status;
    //填充
    uint8 padding2;
};

int start_nproxys();

// int start_tcp_nproxy(struct nproxy_config *cfg);
// int start_udp_nproxy(struct nproxy_config *cfg);

#endif