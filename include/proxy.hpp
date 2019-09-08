#ifndef INCLUDE_PROXY
#define INCLUDE_PROXY

#include "naivelog.hpp"
#include "naiveconfig.hpp"
#include <unordered_map>
using std::unordered_map;
class naiveconfig;

class proxy
{
public:
    //  uint32 fd_in;
    //  uint32 fd_out;
    naiveconfig *cfg;
    proxy();
    virtual ~proxy() = 0;
    virtual void startproxy() = 0;
    virtual void handle_events(int) = 0;
    virtual int do_accept(int) = 0;
    virtual int do_read(int sockfd, unsigned char *buf, int len) = 0;
    virtual int do_send(int sockfd, unsigned char *buf, int len) = 0;
    virtual int setnonblocking(uint32) = 0;
};

class tcpproxy : public proxy
{
public:
    int epollfd;
    struct epoll_event *events;
    //sockfd_fd_map
    unordered_map<int, int> sfh;

    void epoll_addfd(int fd);
    tcpproxy(naiveconfig *cfg);
    virtual ~tcpproxy();
    virtual void startproxy();
    virtual int setnonblocking(uint32);
    virtual void handle_events(int);
    virtual int do_accept(int);
    virtual int do_read(int sockfd, unsigned char *buf, int len);
    virtual int do_send(int sockfd, unsigned char *buf, int len);
};

#endif