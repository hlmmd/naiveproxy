TARGET = ipproxy
CC = gcc

all: udp_client udp_server $(TARGET) 
udp_client: udp_client.c
	$(CC) -o udp_client udp_client.c
udp_server:udp_server.c
	$(CC) -o udp_server udp_server.c
$(TARGET) : config.c LinkList.c LinkList.h  udp.c Clist.c tcpproxy.c
	$(CC) -o $(TARGET) config.c LinkList.c  udp.c Clist.c tcpproxy.c
	cat ReadMe.txt
	
.PHONY:clean
clean:
	rm -f  $(TARGET)  udp udp_client udp_server