#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <netinet/in.h>  
#include <sys/socket.h> 
#include <errno.h>   
#include <arpa/inet.h>  //inet_ntoa()
#include <unistd.h>  
#include <fcntl.h>
#include <time.h>
#include<sys/param.h>
#include "LinkList.h"
int now(char* datetime)
{
	time_t timep;
	struct tm *p;
	
	time(&timep);
	p=localtime(&timep);
	sprintf(datetime, "%4d-%02d-%02d %02d:%02d:%02d", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	return 0;
}

int forkfunc(int accept_sockfd, struct sockaddr_in* server_addr, struct sockaddr_in* client_addr, char* begint)
{
	int conn_sockfd;
	fd_set rfds, wfds, arfds, awfds;
	int retval;
	struct timeval  tv;
	tv.tv_sec = 5; 
	tv.tv_usec = 0;
	int maxfd;
	int flags;
	int retconn;
	char buf1[1000];
	int nlen1;
	char buf2[1000];
	int nlen2;
	int nflag1, nflag2;
	int recesum1 = 0;
	int recesum2 = 0;
	char endt[20];
	int fd;
	char log1[100];
	char log2[100];
	struct flock lock; 
	
	lock.l_type = F_WRLCK;
	lock.l_whence = 0;
	lock.l_start = 0; 
	lock.l_len = 0; 
	
	if((fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND)) < 0){
		printf("open log file error\n");
		exit(-1);
	}
	
	while(fcntl(fd, F_SETLK, &lock) < 0)
	{
		usleep(100);
		continue;
	}
	sprintf(log1, "%s | %s:%d ===> %s:%d\n", begint, inet_ntoa((*client_addr).sin_addr), ntohs((*client_addr).sin_port), inet_ntoa((*server_addr).sin_addr), ntohs((*server_addr).sin_port));
	write(fd, log1, strlen(log1));
	fcntl(fd,F_UNLCK,0);
	fsync(fd);

	if((conn_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		printf("create socket error!\n");
		exit(-1);
	}
	if((flags = fcntl(conn_sockfd, F_GETFL, 0)) < 0){
		printf("fcntl getfl error\n");
		exit(-1);
	}
	flags |= O_NONBLOCK;
	if(fcntl(conn_sockfd, F_SETFL, flags) < 0){
		printf("fcntl setfl error\n");
		exit(-1);
	}
	
	if((retconn = connect(conn_sockfd, (struct sockaddr*)server_addr, sizeof(*server_addr))) == 0){
			goto done;
	}
	else if(retconn < 0 && errno != EINPROGRESS){
		printf("connect error\n");
		exit(-1);
	}
			
	maxfd = conn_sockfd;
	FD_ZERO(&wfds);
	FD_ZERO(&rfds);
	FD_SET(conn_sockfd, &wfds);
	FD_SET(conn_sockfd, &rfds);
	retval = select(maxfd+1, &rfds, &wfds, NULL, &tv);
	if(retval == 0){
		printf("select run out time\n");
		exit(-1);
	}
	else if(retval < 0){
		printf("select error\n");
		exit(-1);
	}

done:
	//printf("conn succ\n");
	FD_ZERO(&awfds);
	FD_ZERO(&arfds);
	FD_SET(conn_sockfd, &awfds);
	FD_SET(conn_sockfd, &arfds);
	FD_SET(accept_sockfd, &awfds);
	FD_SET(accept_sockfd, &arfds);
	maxfd = conn_sockfd>accept_sockfd ? conn_sockfd : accept_sockfd; 
	nflag1 = 0;
	nflag2 = 0;
	while(1){
		rfds = arfds;
		wfds = awfds;
		retval = select(maxfd+1, &rfds, &wfds, NULL, &tv);
		if(retval < 0){
			printf("select error\n");
			exit(-1);
		}
		else if(retval == 0)
			continue;
		if(FD_ISSET(accept_sockfd, &rfds)){
			if(nflag1 == 0){
				nlen1 = recv(accept_sockfd, buf1, 1000, 0);
				//printf("acc read: %s\n", buf1);
				nflag1 = 1;
				recesum1 += nlen1;
				if(nlen1 == 0){
					close(accept_sockfd);
					break;
				}
			}
		}
		if(FD_ISSET(conn_sockfd, &wfds)){
			if(nflag1 == 1){
				send(conn_sockfd, buf1, nlen1, 0);
				//printf("send to neiwang\n");
				nflag1 = 0;
			}
		}
		if(FD_ISSET(conn_sockfd, &rfds)){
			if(nflag2 == 0){
				nlen2 = recv(conn_sockfd, buf2, 1000, 0);
				//printf("conn read: %s\n", buf2);
				nflag2 = 1;
				recesum2 += nlen2;
				if(nlen2 == 0){
					close(accept_sockfd);
					break;
				}
			}
		}
		if(FD_ISSET(accept_sockfd, &wfds)){
			if(nflag2 == 1){
				nlen2 = send(accept_sockfd, buf2, nlen2, 0);
				nflag2 = 0;
			}
		}
	}
	close(conn_sockfd);
	now(endt);
	while(fcntl(fd, F_SETLK, &lock) < 0)
	{
		usleep(100);
		continue;
	}
	
	endt[19] = '\0';
	sprintf(log2, "%s | %s:%d ---> %s:%d(%dBytes) | %s:%d <--- %s:%d(%dBytes)\n", endt, inet_ntoa((*client_addr).sin_addr), ntohs((*client_addr).sin_port), inet_ntoa((*server_addr).sin_addr), ntohs((*server_addr).sin_port), recesum1, inet_ntoa((*client_addr).sin_addr), ntohs((*client_addr).sin_port), inet_ntoa((*server_addr).sin_addr), ntohs((*server_addr).sin_port), recesum2);
	write(fd, log2, strlen(log2));
	fcntl(fd,F_UNLCK,0);
	fsync(fd);
	close(fd);
	return 0;
}

int tcpfunc(unsigned short Io_INOUT, unsigned short listen_port, unsigned int ip, unsigned short port)
{
	int listen_sockfd;
	int accept_sockfd;
	struct sockaddr_in host_addr;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int socklen;
	
	pid_t pid;
	
	int flags;
	char buf[2000];
	int nlen;
	
	char begint[20];
	int on = 1;
	
	memset((void*)&host_addr, 0, sizeof(host_addr));
	memset((void*)&server_addr, 0, sizeof(server_addr));
	memset((void*)&client_addr, 0, sizeof(client_addr));
	
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(listen_port);
	if(Io_INOUT == IO_INOUT)
		inet_aton(MY_IN_IP , (struct in_addr*)&host_addr.sin_addr);
	else
		inet_aton(MY_OUT_IP, (struct in_addr*)&host_addr.sin_addr);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	//server_addr.sin_addr.s_addr = htonl(ip);
	server_addr.sin_addr.s_addr = ip;
	//inet_aton(ip, (struct in_addr*)&server_addr.sin_addr);
	
	if((listen_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		printf("create socket error!\n");
		exit(-1);
	}
	
	if(setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) { 
		perror("setsockopt"); 
		exit(1);   
	} 
	
	if((flags = fcntl(listen_sockfd, F_GETFL, 0)) < 0){
		printf("fcntl getfl error\n");
		exit(-1);
	}
	flags |= O_NONBLOCK;
	if(fcntl(listen_sockfd, F_SETFL, flags) < 0){
		printf("fcntl setfl error\n");
		exit(-1);
	}
	
	if(bind(listen_sockfd, (struct sockaddr*)&host_addr, sizeof(host_addr)) < 0){
		printf("bind error\n");
		exit(-1);
	}
	
	if(listen(listen_sockfd, 500) < 0){
		printf("listen error\n");
		exit(-1);
	}
	
	while(1)
	{	
		socklen = sizeof(client_addr);
		if((accept_sockfd = accept(listen_sockfd, (struct sockaddr*)&client_addr, &socklen)) < 0){
			continue;
		}
		
		now(begint);
		begint[19] = '\0';
		
		//printf("accept succ\n");
		signal(SIGCHLD,SIG_IGN);
		if((pid = fork()) < 0){
			printf("fork error\n");
			exit(-1);
		}
		else if(pid == 0){
			forkfunc(accept_sockfd, &server_addr, &client_addr, begint);
			close(listen_sockfd);
			//printf("proc end\n");
			exit(0);
		}
		close(accept_sockfd);
	}
	
	close(listen_sockfd);
	return 0;
}

