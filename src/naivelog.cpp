#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "naivelog.hpp"

#define USE_O_APPEND

naivelog *naivelog::nlog = NULL;

naivelog::~naivelog()
{
    enabled = true;
    close(logfd);
}

naivelog::naivelog()
{
//    int fd = -1;
//初始化 logfd
//当有O_CREAT时，需要使用三个参数。
//使用了O_APPEND标志位时，write是原子操作，可以不加锁
#ifdef USE_O_APPEND
    if ((logfd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT | O_APPEND, 0755)) < 0)
#else
    if ((logfd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT, 0755)) < 0)
#endif
    {
        printf("open log file error\n");
        exit(-1);
    }
}

void naivelog::enable()
{
    enabled = true;
}

void naivelog::disable()
{
    enabled = false;
}

naivelog *naivelog::GetInstance()
{
    if (nlog == NULL)
        nlog = new naivelog();

    return nlog;
}

int naivelog::get_currenttime(char *datetime)
{
    time_t timep;
    struct tm *p;

    time(&timep);
    p = localtime(&timep);
    sprintf(datetime, "%4d-%02d-%02d %02d:%02d:%02d: ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    return 0;
}

int naivelog::write_log(char *oristr, int length)
{
    if (enabled == false)
        return -1;
    char str[1024];
    memset(str, 0, sizeof(str));
    get_currenttime(str);

    length += strlen(str);
    strcat(str, oristr);

    int ret = 0;

    //日志文件极大地影响了性能。
    //TODO: 改成mmap

#ifdef USE_O_APPEND
    ret = write(logfd, str, length);
    // fsync(fd);
#else
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    //lock.l_pid = getpid();
    do
    {
        ret = fcntl(logfd, F_SETFL, &lock);
    } while (ret < 0);

    ret = write(logfd, str, length);

    fsync(logfd);
    fcntl(logfd, F_UNLCK, 0);
#endif
    return ret;
}
