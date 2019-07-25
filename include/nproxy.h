#ifndef INCLUDE_NPROXY
#define INCLUDE_NPROXY

#define CONFIG_FILE_NAME "/etc/naiveproxy.conf"

#include "nproxy-config.h"
#include "hashmap.h"
//设置4字节对齐
#pragma pack(4)

// struct nproxy
// {
//     //指针，指向进程对应的config
//     struct nproxy_config *cfg;
//     //client到nproxy的连接的socket
//     uint32 client_fd;
//     //nproxy到dest的连接的socket
//     uint32 dest_fd;
//     // //状态
//     // uint8 status;
//     // //填充
//     // uint8 padding2[3];
// };

int open_only_once();
int start_nproxys();
int daemonize();
// int start_tcp_nproxy(struct nproxy_config *cfg);
// int start_udp_nproxy(struct nproxy_config *cfg);

#endif