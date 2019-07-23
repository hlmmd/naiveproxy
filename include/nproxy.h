#ifndef INCLUDE_NPROXY
#define INCLUDE_NPROXY

#include "nproxy-config.h"

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
    //IO类型 IN/OUT/INOUT
    uint8 io_type;
    //协议类型 TCP or UDP
    uint8 protocol;
    //填充字节
    uint16 padding;
    //用户IP地址
    uint32 client_ipaddr;
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

int start_tcp_nproxy(short io_type,
                     int proxy_ipaddr, //proxy server ip
                     short proxy_port, //proxy service port
                     int dest_ipaddr,
                     short dest_port);

#endif