#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <fcntl.h>

#define MAX_LINE 1600

#define END 1000
int udpproxy(unsigned short inout, unsigned short listen_port, unsigned int des_ip, unsigned short port);