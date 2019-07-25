#!/bin/bash


# 安装naiveproxy

rm -rf build/
mkdir build
cd build
cmake ../
make
sudo make install
cd ..

# 添加到service列表中

#添加service 脚本
# echo "#!/bin/bash
# start() 
# {
#     /usr/local/bin/naiveproxy
# }

# stop() 
# {
#     kill -9 $(pidof naiveproxy)	
# }

# case "$1" in
# start)
#     start
#     ;;

# stop)
#     stop
#     ;;

# restart)
#     stop
#     start
#     ;;

# *)
#     echo \"Usage: $0 {start|stop|restart}\"
#     exit 0
#     ;;

# esac
# exit 0" >/etc/init.d/naiveproxy1

# #设置可执行权限
# sudo chmod a+x /etc/init.d/naiveproxy1


# [Unit]
# Description=naiveproxy server
# After=

# [Service]
# Type=forking
# PIDFile=/run/naiveproxy.pid
# #ExecStartPre=/usr/sbin/nginx -t
# ExecStart=naiveproxy
# ExecReload=killall -9 naiveproxy && naiveproxy
# ExecStop=killall -9 naiveproxy
# PrivateTmp=true

# [Install]
# WantedBy=multi-user.target