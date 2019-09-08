#ifndef INCLUDE_NAIVELOG
#define INCLUDE_NAIVELOG

#define LOG_FILE_NAME "/tmp/naiveproxy.log"

class naivelog
{
    bool enabled;
    static naivelog *nlog;
    int logfd;
    naivelog();
    ~naivelog();

public:
    void enable();
    void disable();
    static naivelog *GetInstance();
    int write_log(char *oristr, int length);
    int get_currenttime(char *datetime);
};

#endif