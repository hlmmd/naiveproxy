#include <ifaddrs.h>
#include "udp.h"

int main(int argc, char** argv)
{
	char my_IP[20];
	int port;
	int sockfd, ret;
	struct sockaddr_in myaddr;
	struct sockaddr_in clientaddr;
	socklen_t addr_length = sizeof(clientaddr);
	char sendbuff[MAX_LINE + 1];
	char recvbuff[MAX_LINE + 1];
	int flags;
	int flag = 1, len = sizeof(int);
	fd_set rds;
	struct timeval timeout;
	struct ifaddrs * ifAddrStruct = NULL;
	void * tmpAddrPtr = NULL;
    
	timeout.tv_sec  = 1;  
	timeout.tv_usec = 0;

	if (argc < 2) 
	{
		printf("input error\n");
		exit(0);
	}

	port = atoi(argv[1]);
	if (port < 0 || port > 65535)
	{
		printf("illegal port!\n");
		exit(0);
	}

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	getifaddrs(&ifAddrStruct);

	while (ifAddrStruct!=NULL) 
	{
		if (ifAddrStruct->ifa_addr->sa_family == AF_INET) 
		{
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
            		inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if (strcmp(addressBuffer, "127.0.0.1") != 0)
				strcpy(my_IP, addressBuffer);
		}
		ifAddrStruct = ifAddrStruct->ifa_next;
	}

	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) == -1) 
	{ 
		perror("setsockopt"); 
		exit(1); 
	}

	if (bind(sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr)) == -1)
	{
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	while(1)
	{
		FD_ZERO(&rds);
		FD_SET(sockfd, &rds);
		ret = select(sockfd + 1, &rds, NULL, NULL, &timeout);
        
		if (ret == -1) 
		{
			printf("select error\n");
			exit(-1);
		}

		if (ret == 0) 
		{
			continue;;
		}

		if (FD_ISSET(sockfd, &rds)) 
		{
			if ((ret = recvfrom(sockfd, recvbuff, MAX_LINE, 0, (struct sockaddr *)&clientaddr, &addr_length)) < 0) 
			{
				printf("recvfrom msg error: %s(errno: %d)\n", strerror(errno), errno);
				exit(1); 
			}
			recvbuff[ret] = 0;
			printf("Recv From Client: %s\n", recvbuff);
			memset(sendbuff, 0, sizeof(sendbuff));
			sprintf(sendbuff, "Hello Client(Send From %s)", my_IP);
			if ((ret = sendto(sockfd, sendbuff, strlen(sendbuff), 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr))) < 0)
			{
				printf("sendto msg error: %s(errno: %d)\n", strerror(errno), errno);
				exit(1);
			}
		}
	}
	close(sockfd);
	return 0;
}