#ifndef INCLUDE_PROXY
#define INCLUDE_PROXY

#include "naiveconfig.hpp"

class naiveconfig;

class proxy
{
public:
    uint32 fd_in;
    uint32 fd_out;
    naiveconfig *cfg;
    proxy();
    virtual ~proxy() = 0;
    virtual void startproxy() = 0;
    virtual int setnonblocking(uint32) = 0;
};

class tcpproxy : public proxy
{
public:
    tcpproxy(naiveconfig *cfg);
    virtual ~tcpproxy();
    virtual void startproxy();
    virtual int setnonblocking(uint32);
    void epoll_addfd(int eopllfd,int fd);
    
};

#endif