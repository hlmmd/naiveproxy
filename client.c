#define _GNU_SOURCE 1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#define BUFFER_SIZE 64

int setnonblocking(int fd)
{
	int old_opt = fcntl(fd, F_GETFL);
	int new_opt = old_opt | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_opt);
	return old_opt;
}

int main(int argc, char **argv)
{
	if (argc <= 2)
	{
		printf("usage: ./%s ip_address port \n", basename(argv[0]));
		return 1;
	}

	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(-1);
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], (struct in_addr *)&server_addr.sin_addr.s_addr) == 0)
	{
		perror(argv[1]);
		exit(errno);
	}

	setnonblocking(sockfd);

	connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (errno != EINPROGRESS)
	{
		perror("connect");
		exit(errno);
	}
	printf("connected server !\n");

	fd_set rfds;
	int maxfd,ret;
	struct timeval tv;
	char buffer[BUFFER_SIZE];
	while (1)
	{

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		maxfd = 0;
		FD_SET(sockfd, &rfds);
		if (sockfd > maxfd)
			maxfd = sockfd;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		//select
		ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
		if (ret == -1)
		{
			printf("select error\n");
			break;
		}
		else if (ret == 0)
			continue;
		else
		{
			if (FD_ISSET(sockfd, &rfds))
			{ 
				bzero(buffer, BUFFER_SIZE);
				int len = recv(sockfd, buffer, BUFFER_SIZE, 0);
				if (len > 0)
					printf("recv from server: %s\n", buffer);
				else if (len < 0)
					printf("receive failed!\n");
				else
				{
					printf("server down\n");
					break;
				}
			}
			if (FD_ISSET(0, &rfds))
			{ 
				bzero(buffer, BUFFER_SIZE);
				fgets(buffer, BUFFER_SIZE, stdin);
				buffer[strlen(buffer) - 1] = '\0';

				int len = send(sockfd, buffer, strlen(buffer), 0);
				if (len < 0)
				{
					printf("send error\n");
					break;
				}
				else if (len != 0)
					printf("send msg: %s \n", buffer);
			}
		}
	}

	close(sockfd);
	return 0;
}