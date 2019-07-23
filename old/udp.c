#include "Clist.h"
#include "udp.h"
#include "LinkList.h"
#include <time.h>

int udpproxy(unsigned short inout, unsigned short listen_port, unsigned int des_ip, unsigned short d_port)
{
	int fd;
	struct flock lock; 
	unsigned short IO_type = inout;
	int port;
	int client_num, i;
	int recvfd, sendfd, maxfd, ret;
	unsigned my_port =listen_port;
	unsigned long serv_ip_addr = des_ip;
	unsigned int des_port =  d_port;
	int traffic, traffic_recv, traffic_send;
	struct sockaddr_in recvaddr;
	struct sockaddr_in sendaddr;
	struct sockaddr_in servaddr;
	struct sockaddr_in clientaddr;
	struct sockaddr_in emptyaddr;
	socklen_t addr_length = sizeof(clientaddr);
	fd_set rds;
	int bufsize = 32 * 1024;
	char    recvbuff[MAX_LINE + 1];
	char    sendbuff[MAX_LINE + 1];
	int flags;
	int flag = 1, len = sizeof(int);
	Clist Client_List;
	ClistNode E;
	struct tm tmnow;
	struct timeval st;
	char time_s[20];
	int time_sec;
	clock_t start, end;

	struct timeval timeout;

	timeout.tv_sec  = 1;
	timeout.tv_usec = 0;

	lock.l_type = F_WRLCK;
	lock.l_whence = 0;
	lock.l_start = 0; 
	lock.l_len = 0; 

	InitList(&Client_List);

	fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND); 
	
	if ((recvfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
	{
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	flags = fcntl(recvfd, F_GETFL, 0);
	fcntl(recvfd, F_SETFL, flags | O_NONBLOCK);

	setsockopt(recvfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

	memset(&recvaddr, 0, sizeof(recvaddr));
	recvaddr.sin_family = AF_INET;
	if (IO_type == IO_OUTIN)
		recvaddr.sin_addr.s_addr = inet_addr(MY_OUT_IP);
	else
		recvaddr.sin_addr.s_addr = inet_addr(MY_IN_IP);
	recvaddr.sin_port = htons(my_port);

	if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) == -1) 
	{ 
		perror("setsockopt"); 
		exit(1); 
	}

	if (bind(recvfd, (struct sockaddr*)&recvaddr, sizeof(recvaddr)) == -1)
	{
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = serv_ip_addr;
	servaddr.sin_port = htons(des_port);

	port = my_port + 1;
	start = clock();
	while (1) 
	{
		FD_ZERO(&rds);
		FD_SET(recvfd, &rds);
		maxfd = recvfd;
		client_num = ListLength(Client_List);
		end = clock();
		if ((end - start) >= 1000)
		{
			start = clock();
			for (i = 1; i <= client_num; i++)
			{
				time_sec = ListGetTimeA(Client_List, i);
				ListSetTimeA(&Client_List, i, ++time_sec);
				if (time_sec >= END)
				{
					while(fcntl(fd, F_SETLK, &lock) < 0)
					{
						usleep(100);
						continue;
					}
					gettimeofday(&st, NULL);
					localtime_r(&st.tv_sec, &tmnow);
					sprintf(time_s, "%04d-%02d-%02d %02d:%02d:%02d",tmnow.tm_year+1900,tmnow.tm_mon+1,tmnow.tm_mday,tmnow.tm_hour,tmnow.tm_min,tmnow.tm_sec);
					ListGetRecvTraffic(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), &traffic_recv);
					ListGetSendTraffic(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), &traffic_send);
					sprintf(sendbuff, "%s | %s:%d ---> %s:%d(%dBytes) | %s:%d <--- %s:%d(%dBytes)\n", time_s, 
						inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), traffic_recv, 
						inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), traffic_send);
					write(fd, sendbuff, strlen(sendbuff));
					fcntl(fd,F_UNLCK,0);
					fsync(fd);
					
					ListGetSendfd(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), &sendfd);
					close(sendfd);
					ListDelete(&Client_List, LocateElem(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port)));
					client_num--;
					continue;
				}
			}
		}
		for (i = 1; i <= client_num; i++)
		{
			sendfd = ListGetSendfdA(Client_List, i);
			FD_SET(sendfd, &rds);
			maxfd = sendfd > maxfd ? sendfd : maxfd;
		}
		ret = select(maxfd + 1, &rds, NULL, NULL, &timeout);
		if (ret == -1) 
		{
			printf("select error\n");
			exit(-1);
		}

		else if (ret == 0) 
		{
			continue;
		}

		if (FD_ISSET(recvfd, &rds))
		{
			if ((ret = recvfrom(recvfd, recvbuff, MAX_LINE, MSG_PEEK, (struct sockaddr*)&clientaddr, &addr_length)) < 0)
			{
				printf("recvfrom msg error: %s(errno: %d)\n", strerror(errno), errno);
				close(ListGetSendfdA(Client_List, LocateElem(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port))));
				ListDelete(&Client_List, LocateElem(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port)));
				continue;
			}
			if (IfExists(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port)) == FALSE)
			{
				ret = recvfrom(recvfd, recvbuff, MAX_LINE, 0, (struct sockaddr*)&clientaddr, &addr_length);

				while(fcntl(fd, F_SETLK, &lock) < 0)
				{
					usleep(100);
					continue;
				}
				gettimeofday(&st, NULL);
				localtime_r(&st.tv_sec, &tmnow);
				sprintf(time_s, "%04d-%02d-%02d %02d:%02d:%02d",tmnow.tm_year+1900,tmnow.tm_mon+1,tmnow.tm_mday,tmnow.tm_hour,tmnow.tm_min,tmnow.tm_sec);
				sprintf(sendbuff, "%s | %s:%d ===> %s:%d\n", time_s, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
				write(fd, sendbuff, strlen(sendbuff));
				fcntl(fd,F_UNLCK,0);
				fsync(fd);

				ListInsert(&Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port));
				ListSetRecvTraffic(&Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), ret);

				if ((sendfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
				{
					printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
					exit(0);
				}

				flags = fcntl(sendfd, F_GETFL, 0);
				fcntl(sendfd, F_SETFL, flags | O_NONBLOCK);

				setsockopt(sendfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

				memset(&sendaddr, 0, sizeof(sendaddr));
				sendaddr.sin_family = AF_INET;
				if (IO_type == IO_OUTIN)
					sendaddr.sin_addr.s_addr = inet_addr(MY_IN_IP);
				else
					sendaddr.sin_addr.s_addr = inet_addr(MY_OUT_IP);
				sendaddr.sin_port = htons((port++)%65536);

				if (setsockopt(sendfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) == -1) 
				{ 
					perror("setsockopt"); 
					exit(1);
				}

				if (bind(sendfd, (struct sockaddr*)&sendaddr, sizeof(sendaddr)) == -1)
				{
					printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
					exit(0);
				}
				
				ret = sendto(sendfd, recvbuff, ret, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
				ListSetSendfd(&Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), sendfd);
			}
			else
			{
				ret = recvfrom(recvfd, recvbuff, MAX_LINE, 0, (struct sockaddr*)&clientaddr, &addr_length);
				ListSetTime(&Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), 0);
				ListGetRecvTraffic(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), &traffic);
				ListSetRecvTraffic(&Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), (traffic + ret));
				ListGetSendfd(Client_List, ntohl(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port), &sendfd);
				sendto(sendfd, recvbuff, ret, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
			}
		}

		for (i = 1; i <= client_num; i++)
		{		
			sendfd = ListGetSendfdA(Client_List, i);
			if (FD_ISSET(sendfd, &rds))
			{
				if ((ret = recvfrom(sendfd, recvbuff, MAX_LINE, MSG_PEEK, (struct sockaddr*)&emptyaddr, &addr_length)) < 0)
				{
					printf("recvfrom msg error: %s(errno: %d)\n", strerror(errno), errno);
					exit(1);
				}
				else
				{
					ret = recvfrom(sendfd, recvbuff, MAX_LINE, 0, (struct sockaddr*)&emptyaddr, &addr_length);
					GetElem(Client_List, i, &E);
					ListGetSendTraffic(Client_List, E.IPaddr, E.port, &traffic); 
					ListSetSendTraffic(&Client_List, E.IPaddr, E.port, (traffic + ret));
					memset(&clientaddr, 0, sizeof(clientaddr));
					clientaddr.sin_family = AF_INET;
					clientaddr.sin_addr.s_addr = htonl(E.IPaddr);
					clientaddr.sin_port = htons(E.port);
					sendto(recvfd, recvbuff, ret, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
				}
			}
		}
	}
	close(fd);
	DestoryList(&Client_List);
	close(recvfd);
	return 0;
}