# naiveproxy

一个基于C语言的proxy程序。

## 功能

通过代理服务器的流量转发功能，实现内外网通信。

目前支持的协议：TCP

## 使用方法

安装：

```bash
mkdir build
cd build
cmake ../
make
sudo make install
sudo touch /etc/naiveproxy.conf
```

使用：配置naiveproxy.conf文件

```
sudo vi /etc/naiveproxy.conf
#参数说明：
#协议类型，目前只支持TCP
#IOTYPE: 内网到外网OUTIN或者外网到内网INOUT
#proxy服务器IP地址
#proxy服务器端口
#目的IP地址
#目的端口
#例子：外网想通过proxy服务器(192.168.3.90)的6666号端口访问内网(192.168.100.233)的80端口（web服务）
TCP OUTIN   192.168.3.90    6666    192.168.100.233 80
```

关闭进程：

```
killall -9 naiveproxy
```

## 技术原理

基本流程：创建守护进程，读取配置文件，每一条配置fork一个子进程进行处理。

每一个子进程accept对应proxy server IP和port上的连接请求，并根据其destination ip和port向目的地发出connect请求，建立两个连接。

多个连接使用**epoll**IO复用模型，ET工作模式。

两个连接对应两个socket，使用哈希表进行映射，当一个socket可读的时候，读取数据，然后通过查询哈希表得到对端的socket，写入读到的数据。

## TODO LIST

* 扩充协议：UDP、HTTP等
* 完善日志系统
* 添加信号处理
* 零拷贝技术
* 细节处理
* 兼容性问题
* 完善安装脚本
* ……