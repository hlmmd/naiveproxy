#ifndef INCLUDE_NAIVEPROXY
#define INCLUDE_NAIVEPROXY

#include "proxy.hpp"
#include <list>
using std::list;

class proxy;

class naiveconfig;

//单利模式 工厂， 读取config并创建proxy实例。
class naiveproxy{
    public:
    //获取对象指针
    static naiveproxy* GetInstance();
    static void DestroyInstance();

    //设置守护进程
    int daemonize();

    //设置进一个进程实例
    int open_only_once();
    

    //启动一个proxy
    int StartProxy(proxy*);

    int StopProxy(proxy *);

    int init_proxys();

    protected:

    private:
    //对象实例指针
    static naiveproxy * instance;
    naiveproxy();
    ~naiveproxy();

    //是否设置为守护进程
    bool daemonized ;
    //打开一个文件，保证只有一个naiveproxy程序实例，初始化为-1
    int oncefd;


    //配置文件
    list<naiveconfig*> cfgs;

};



#endif