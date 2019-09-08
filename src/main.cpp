#include <stdio.h>
#include <stdlib.h>

#include "naiveproxy.hpp"

int main()
{
	naiveproxy *np = naiveproxy::GetInstance();

	//设置守护进程
	np->daemonize();

	//初始化日志
	naivelog::GetInstance();

	//执行proxy
	np->init_proxys();

	while (1)
		;

	naiveproxy::DestroyInstance();

	return 0;
}