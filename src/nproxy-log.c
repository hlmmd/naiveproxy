
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "nproxy-log.h"
#include "nproxy.h"

#define USE_O_APPEND

int get_currenttime(char *datetime)
{
    time_t timep;
    struct tm *p;

    time(&timep);
    p = localtime(&timep);
    sprintf(datetime, "%4d-%02d-%02d %02d:%02d:%02d: ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    return 0;
}

int init_log()
{
    int fd;
    //当有O_CREAT时，需要使用三个参数。
    //使用了O_APPEND标志位时，write是原子操作，可以不加锁
#ifdef USE_O_APPEND
    if ((fd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT | O_APPEND, 0755)) < 0)
#else
    if ((fd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT, 0755)) < 0)
#endif
    {
        printf("open log file error\n");
        exit(-1);
    }

    return fd;
}

int nproxy_log(int fd, char *oristr, int length)
{

    char str[1024];
    memset(str, 0, sizeof(str));
    get_currenttime(str);

    length += strlen(str);
    strcat(str, oristr);

    if (fd < 0)
        return fd;

    int ret = 0;
#ifdef USE_O_APPEND
    ret = write(fd, str, length);
    fsync(fd);
#else
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    //lock.l_pid = getpid();
    do
    {
        ret = fcntl(fd, F_SETFL, &lock);
    } while (ret < 0);

    ret = write(fd, str, length);

    fsync(fd);
    fcntl(fd, F_UNLCK, 0);
#endif
    return ret;
}

int destroy_log(int fd)
{
    if (fd)
        close(fd);
}