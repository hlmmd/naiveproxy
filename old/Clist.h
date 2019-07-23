#define TRUE		1
#define FALSE		0
#define OK		1
#define ERROR		0
#define INFEASIBE	-1
#define OVERFLOW	-2

typedef struct ClistNode
{
	int sendfd;
	long IPaddr;
	int port;
	int recv_traffic;
	int send_traffic;
	int time_sec;
	struct ClistNode* next;
}ClistNode, *Clist;

typedef int Status;

Status	InitList(Clist *L);
Status	DestoryList(Clist *L);
int 	LocateElem(Clist L, long IPaddr, int port);
int 	ListLength(Clist L);
Status  IfExists(Clist L, long IPaddr, int port);
Status	ListInsert(Clist *L, long IPaddr, int port);
Status	ListDelete(Clist *L, int i);
Status	ListSetRecvTraffic(Clist *L, long IPaddr, int port, int recv_traffic);
Status	ListGetRecvTraffic(Clist L, long IPaddr, int port, int *recv_traffic);
Status	ListSetSendTraffic(Clist *L, long IPaddr, int port, int send_traffic);
Status	ListGetSendTraffic(Clist L, long IPaddr, int port, int *send_traffic);
Status	ListSetSendfd(Clist *L, long IPaddr, int port, int sendfd);
Status	ListGetSendfd(Clist L, long IPaddr, int port, int *sendfd);
int	ListGetSendfdA(Clist L, int i);
Status	ListSetTime(Clist *L, long IPaddr, int port, int time_sec);
Status	ListGetTime(Clist L, long IPaddr, int port, int *time_sec);
int	ListGetTimeA(Clist L, int i);
int	ListSetTimeA(Clist *L, int i, int time_sec);

