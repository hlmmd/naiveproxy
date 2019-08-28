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

naiveproxy::naiveproxy()
{
    printf("constructor\n");
    daemonized = false;
    oncefd = -1;

}

naiveproxy::~naiveproxy()
{
    printf("destructor\n");
    //关闭打开文件
    if(oncefd!=-1)
        close(oncefd);
}

naiveproxy *naiveproxy::instance = NULL;

naiveproxy *naiveproxy::GetInstance()
{
    if (instance == NULL)
    {
        instance = new naiveproxy();
    }
    return instance;
}

void naiveproxy::DestroyInstance()
{
    if( instance!=NULL)
    {
        delete instance;
        instance = NULL;
    }
}

int naiveproxy::daemonize()
{
    if(daemonized == true)
        return -1;

    int pid;
    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
        exit(1);
    //创建会话期
    setsid();

    //fork 两次
    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
        exit(1);

    //设置工作目录，设置为/tmp保证具有权限
    int ret = chdir("/tmp");
    if (ret != 0)
    {
        return -1;
    }

    //设置权限掩码
    umask(0);

    //关闭已经打开的文件描述符
    //for (int i = 0; i < getdtablesize(); i++)
    for (int i = 0; i < 2; i++)
        close(i);

    //忽略SIGCHLD信号，防止产生僵尸进程
    signal(SIGCHLD, SIG_IGN);

    daemonized  = true;
    return 0;
}

int naiveproxy::open_only_once()
{
    const char filename[] = "/tmp/naiveproxy.pid";
    int fd, val;
    char buf[10];
    //打开控制文件，控制文件打开方式：O_WRONLY | O_CREAT只写创建方式
    //控制文件权限：S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH用户、用户组读写权限
    if ((fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
        return -1;
    }
    // try and set a write lock on the entire file
    struct flock lock;
    //建立一个供写入用的锁
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    //以文件开头为锁定的起始位置
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    //结合lock中设置的锁类型，控制文件设置文件锁，此处设置写文件锁
    if (fcntl(fd, F_SETLK, &lock) < 0)
    {
        //如果获取写文件锁成功，则退出当前进程，保留后台进程
        if (errno == EACCES || errno == EAGAIN)
        {
            printf("naiveproxy has already run.\n");
            exit(-1); // gracefully exit, daemon is already running
        }
        else
        {
            //   printf("file being used\n");
            return -1; //如果锁被其他进程占用，返回 -1
        }
    }
    // truncate to zero length, now that we have the lock
    //改变文件大小为0
    if (ftruncate(fd, 0) < 0)
        return -1;
    // and write our process ID
    //获取当前进程pid
    sprintf(buf, "%d\n", getpid());
    //将启动成功的进程pid写入控制文件
    if (write(fd, buf, strlen(buf)) != strlen(buf))
        return -1;

    // set close-on-exec flag for descriptor
    // 获取当前文件描述符close-on-exec标记
    if ((val = fcntl(fd, F_GETFD, 0)) < 0)
        return -1;
    val |= FD_CLOEXEC;
    //关闭进程无用文件描述符
    if (fcntl(fd, F_SETFD, val) < 0)
        return -1;
    // leave file open until we terminate: lock will be held

    oncefd = fd;

    return fd;
}