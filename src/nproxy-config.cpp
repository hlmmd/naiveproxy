
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
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/param.h>
#include <sys/select.h>
#include <assert.h>
#include "nproxy.h"
#include "nproxy-config.h"

void print_one_config(struct nproxy_config *cfg)
{
    struct in_addr nproxy_server;
    nproxy_server.s_addr = cfg->proxy_ipaddr;

    struct in_addr dest_server;
    dest_server.s_addr = cfg->dest_ipaddr;
    char dest_ip[128];
    strcpy(dest_ip, inet_ntoa(dest_server));

    //注意，inet_ntoa是不可重入的。
    printf("PROTOCOL:          %s\n"
           "nproxy_server_ip:  %s\n"
           "nproxy_port:       %d\n"
           "dest_ipaddr:       %s\n"
           "dest_port:         %d\n",
           cfg->protocol == PROTOCOL_TCP ? "TCP" : "UDP",
           inet_ntoa(nproxy_server),
           cfg->proxy_port,
           dest_ip,
           cfg->dest_port);
}

char *strupper(char *src)
{
    char *p = src;
    while (*p)
    {
        if (*p >= 'a' && *p <= 'z')
            *p -= 'a' - 'A';
        p++;
    }

    return src;
}

//去除comment，将第一个#置0，返回去除后的字符串长度
int remove_comment(char *str)
{
    int i = 0;
    for (i = 0; str[i]; i++)
    {
        if (str[i] == '#')
        {
            str[i] = 0;
            return i;
        }
    }
    return i;
}

int read_one_config(char *str_ptr, char *out)
{
    int i = 0;
    char *str = str_ptr;
    char *p = out;
    //忽略前导空格
    while (*str == ' ' || *str == '\t' || *str == '\n')
    {
        i++;
        str++;
    }
    for (; *str; i++)
    {
        if (*str == ' ' || *str == '\t' || *str == '\n')
        {
            i++;
            str++;
            break;
        }
        *p = *str++;
        p++;
    }
    *p = 0;
    //  printf("%s\n",out);
    return i;
}

int read_config_line(char *str, struct nproxy_config *cfg)
{
    char *p_str = str;
    char one_conf[128];
    int ret = 0;

    //TCP or UDP
    memset(one_conf, 0, sizeof(one_conf));
    ret = read_one_config(p_str, one_conf);
    p_str += ret;

    strupper(one_conf);
    if (strcmp(one_conf, "TCP") == 0)
        cfg->protocol = PROTOCOL_TCP;
    else if (strcmp(one_conf, "UDP") == 0)
        cfg->protocol = PROTOCOL_UDP;
    else
    {
        printf("config error. support protocol: TCP/UDP");
        exit(0);
    }

    //proxy_server_ip
    memset(one_conf, 0, sizeof(one_conf));
    ret = read_one_config(p_str, one_conf);
    p_str += ret;

    struct in_addr pro_addr;
    if (inet_aton(one_conf, &pro_addr) == 0)
    {
        perror(one_conf);
        exit(errno);
    }
    cfg->proxy_ipaddr = pro_addr.s_addr;

    //proxy_server_port
    memset(one_conf, 0, sizeof(one_conf));
    ret = read_one_config(p_str, one_conf);
    p_str += ret;
    cfg->proxy_port = atoi(one_conf);

    //dest_server_ip
    memset(one_conf, 0, sizeof(one_conf));
    ret = read_one_config(p_str, one_conf);
    p_str += ret;

    if (inet_aton(one_conf, &pro_addr) == 0)
    {
        perror(one_conf);
        exit(errno);
    }
    cfg->dest_ipaddr = pro_addr.s_addr;

    //dest_server_port
    memset(one_conf, 0, sizeof(one_conf));
    ret = read_one_config(p_str, one_conf);
    p_str += ret;
    cfg->dest_port = atoi(one_conf);

    return 0;
}

//初始化config，读取所有配置
int init_nproxy_config(struct nproxy_config **cfgs_ptr)
{
    int nproxy_config_size = 0;
    *cfgs_ptr = (struct nproxy_config *)malloc(sizeof(struct nproxy_config) * MAX_CONFIG_SIZE);
    memset(*cfgs_ptr, 0, sizeof(struct nproxy_config) * MAX_CONFIG_SIZE);

    struct nproxy_config *cfgs = *cfgs_ptr;

    FILE *fp;
    fp = fopen(CONFIG_FILE_NAME, "r");
    if (fp == NULL)
        printf("打开文件%s失败\n", CONFIG_FILE_NAME);

    char str[1024];

    while (fgets(str, sizeof(str), fp) != NULL)
    {
        //删除注释
        remove_comment(str);
        char *p_str = str;
        //删除前导空格等
        while (*p_str == ' ' || *p_str == '\t' || *p_str == '\n')
            p_str++;
        if (p_str[0] == 0)
            continue;
        if (nproxy_config_size >= MAX_CONFIG_SIZE)
            continue;
        struct nproxy_config *cfg = cfgs + nproxy_config_size;
        read_config_line(p_str, cfg);
        nproxy_config_size++;
    }
    return nproxy_config_size;
}

int destroy_nproxy_config(struct nproxy_config **cfgs_ptr)
{
    if (*cfgs_ptr)
        free(*cfgs_ptr);
    *cfgs_ptr = NULL;
    return 0;
}