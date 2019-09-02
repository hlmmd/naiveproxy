#ifndef INCLUDE_NAIVECONFIG
#define INCLUDE_NAIVECONFIG


const int MAX_CONFIG_SIZE =  32;
const int MAX_BUFFER_SIZE = 4096;
const int USER_LIMIT = 20000;

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned short uint16;


class naiveconfig{
    public:
    //用于映射socketfd的哈希表。map[clientfd] = destfd,map[destfd]=clientfd
    //struct socket_fd_hashmap *sfh;

    //proxy服务IP
    uint32 proxy_ipaddr;
    //目的ip地址
    uint32 dest_ipaddr;
    //proxy服务端口
    uint16 proxy_port;
    //目的端口
    uint16 dest_port;
    //监听连接的listen_sockfd
    uint32 listen_sockfd;
    //当前连接个数
    uint32 client_num;
    //日志fd
    uint32 logfd;
    //协议类型 TCP or UDP
    uint8 protocol;
    //填充字节
    uint8 padding[3];

    void remove_comment(char * str);

    naiveconfig(char * str);
    void print();

};








enum protocol_type
{
    PROTOCOL_TCP,
    PROTOCOL_UDP,
};


#endif