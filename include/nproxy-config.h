
#ifndef INCLUDE_NPROXY_CONFIG
#define INCLUDE_NPROXY_CONFIG



typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned short uint16;

//设置4字节对齐
#pragma pack (4)

struct nproxy_config
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
};



#endif