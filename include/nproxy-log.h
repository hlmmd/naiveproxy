#ifndef INCLUDE_NPROXY_LOG
#define INCLUDE_NPROXY_LOG

//#define LOG_FILE_NAME   "/var/log/naiveproxy.log"
#define LOG_FILE_NAME   "/tmp/naiveproxy.log"



int init_log();
int nproxy_log(int fd, char * str,int length);
int destroy_log(int fd);



#endif