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

#include <netdb.h>
#include "udp.h"
#include "LinkList.h"


LinkList List;
void SIGTERM_handler(int sig);
/*
typedef struct Con_Node{
	int ip_addr;			//虚拟机ip地址 
	short my_bind_port;		//监听端口号 
	short des_bind_port;	//虚拟机端口号 
	short IO_type;			//方向 
	short Pro_type;			//协议 
	int Enable;				//工作状态 
	int pid;				//fork产生的子程序的进程号 
}ElemType,*ET;
*/


void signchld_handler(int sig)
{
	while(waitpid(-1,0,WNOHANG)>0)
		;
}
void print(LinkList L)
{
	LinkList p = L->next; 
	struct in_addr t;
	printf("\n协议  方向    监听端口号  虚拟机IP          虚拟机端口号 状态 子进程号\n");
    	while(p) {
			printf("%-6s",p->data.Pro_type==0?"TCP":"UDP");
			printf("%-9s",p->data.IO_type==0?"INOUT":"OUTIN");
			t.s_addr = p->data.ip_addr;
		printf("%-10d %-20s %-10d %-5d %-10d\n",p->data.my_bind_port,inet_ntoa(t),p->data.des_bind_port,p->data.Enable,p->data.pid);
		p = p->next;
	}
}

char * strupr(char *str)
{
   char *p = str;
   while (*p != '\0')
   {
      if(*p >= 'a' && *p <= 'z')
      *p -= 0x20;
      p++;
    }
   return str;
}


int Init_Config()
{
	FILE * fp;
	fp = fopen(OPEN_FILE_NAME,"r");
	if(fp==NULL)
		printf("打开文件%s失败\n",OPEN_FILE_NAME);
	char str[1024];
	char *p,temp[1000];
	int i;
	int count;
	int pid;
	short my_bind_port_s;		//监听端口号 
	short my_bind_port_e;		//监听端口号 
	short des_bind_port_s;		//虚拟机端口号 
	short des_bind_port_e;		//虚拟机端口号 
	M_DisableList(List);
	ElemType s;

	int right;
	while(fgets(str,sizeof(str),fp)!=NULL)
	{
	 //	printf("%s",str);	//打印原始信息
		p = str;
		count = 0;
		right = 1;
		s.ip_addr = 0;
		s.pid=0;
		s.Enable = S_NEW;//S_NEW表示新的信息，尚未产生进程 
		while(1)
		{
			if(*p=='\r'||*p=='\n'|| *p==0 ||*p=='#' )
				break;
			else if(*p==' '||*p=='\t')
				p++;
			else {
				i=0;
				count++;
				while(*p!='\r' &&*p!='\n' && *p!='\t' &&*p!=' '&& *p!='#' && *p!='\0')
				{
					temp[i++]=*p;
					p++;
				}
				temp[i] = '\0';
				if(count==1)
				{
					strupr(temp);	//转大写
					if(strcmp(temp,"TCP")==0)
						s.Pro_type = PRO_TCP;
					else if(strcmp(temp,"UDP")==0)
						s.Pro_type = PRO_UDP;
					else{
							right = 0;
							break;
						}
				}		
				else if(count==2)
				{
					strupr(temp);	//转大写
					if(strcmp(temp,"INOUT")==0)
						s.IO_type = IO_INOUT;
					else if(strcmp(temp,"OUTIN")==0)
						s.IO_type = IO_OUTIN;
					else{
							right = 0;
							break;
						}
				}
				else  if(count==3)
				{
					char num1[6];
					for(i=0;temp[i] || temp[i]!='-';i++)
					{
						if(temp[i]>='0'&&temp[i]<='9')
							num1[i]=temp[i];
						else
							break;
					}
					if(temp[i]<'0'&&temp[i]>'9'&&temp[i]!=0 &&temp[i]!='-'){
							right =0;
							break;
						}
					num1[i] = 0;
					my_bind_port_s = atoi(num1);
					if(temp[i]=='\0')
						my_bind_port_e = 0;
					else{
						i++;
						int j=0;
						while(temp[i])
						{
							if(temp[i]>='0'&&temp[i]<='9')
							{
								num1[j]=temp[i];	
								j++,i++;
							}
							else
								break;
						}
						if(temp[i]){
							right = 0;
							break;
						}
						num1[j] =0;
						my_bind_port_e = atoi(num1);
					}
				}
				else  if(count==4)
				{
					struct in_addr t;
					struct hostent *hostp;	
					char **pp;
					if(inet_aton(temp,&t)==0)
					{
						hostp = gethostbyname(temp);
						if(hostp==NULL){
							right = 0;
							break;
						}
						s.ip_addr =   ( (struct in_addr *)* hostp->h_addr_list)->s_addr;
					}
					else
						s.ip_addr = t.s_addr;
				}
				else if(count==5)
				{
					char num1[6];
					for(i=0;temp[i] || temp[i]!='-';i++)
					{
						if(temp[i]>='0'&&temp[i]<='9')
							num1[i]=temp[i];
						else
							break;
					}
					if(temp[i]<'0'&&temp[i]>'9'&&temp[i]!=0 &&temp[i]!='-'){

							right = 0;
							break;
						}
					num1[i] = 0;
					des_bind_port_s = atoi(num1);
					if(temp[i]=='\0')
						des_bind_port_e = 0;
					else{
						i++;
						int j=0;
						while(temp[i])
						{
							if(temp[i]>='0'&&temp[i]<='9')
							{
								num1[j]=temp[i];	
								j++,i++;
							}
							else
								break;
						}
						if(temp[i]){
							right = 0;
							break;
						}
						num1[j] =0;
						des_bind_port_e = atoi(num1);
					}
				}
			}
		}
		if(count==5 && right)
		{
			int ret;
			if(my_bind_port_e==des_bind_port_e &&des_bind_port_e==0)	//单个端口 
			{
				s.my_bind_port=my_bind_port_s;
				s.des_bind_port = des_bind_port_s;
				if(  (ret=M_LocateElem(List,s))==0 )
					M_ListInsert(&List,M_ListLength(List)+1,s);
				else if(  (ret=M_LocateSameElem(List,s))>0 )
					M_EnableList(List,ret);
			}
			else	//多个端口 
			{
				int n = my_bind_port_e-my_bind_port_s;
				if(n>0&&n==des_bind_port_e-des_bind_port_s)
				{
					for(i=0;i<=n;i++)
					{
						s.my_bind_port=my_bind_port_s+i;
						s.des_bind_port = des_bind_port_s+i;
						if(  (ret=M_LocateElem(List,s))==0 )
							M_ListInsert(&List,M_ListLength(List)+1,s);
						else if(  (ret=M_LocateSameElem(List,s))>0 )
							M_EnableList(List,ret);
					}
				}
			}
		}
	}
}


void init_daemon()
{
	int pid;
	int i;
	pid=fork();
	if(pid<0)    
		exit(1);  
	else if(pid>0)
		exit(0);
    
	setsid(); 
	pid=fork();
	if(pid>0)
		exit(0);
	else if(pid<0)    
		exit(1);

	for(i=3;i<NOFILE;i++)
		close(i);
	umask(0);
}


void CreateNewConnect(LinkList L)
{
	struct flock lock;	//文件锁
	lock.l_type = F_WRLCK;
	lock.l_whence = 0;
	lock.l_start = 0; 
	lock.l_len = 0; 
	char time_s[30];
	char sendbuff[200];
	int fd ; //文件描述符
	fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND); 
	time_t rawtime;
	struct tm * ptm;
	struct in_addr t;

	
	int pid;
	LinkList p = L->next; 
   	 ET e;
   	 while(p) {
		
    		if( p->data.Enable==S_NEW || kill(p->data.pid,0)!=0  )
		{
			while(fcntl(fd, F_SETLK, &lock) < 0)
				{
					printf("aaa\n");
					usleep(100);
					continue;
				}
			time ( &rawtime );
			ptm = gmtime(&rawtime);
			sprintf( time_s,"%4d-%02d-%02d %02d:%02d:%02d",ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
			t.s_addr = p->data.ip_addr;
		sprintf(sendbuff,"%s: 【服务启动】%s %s 绑定端口号：%5d 目标IP地址: %15s 目标端口:%5d\n",time_s,p->data.Pro_type==0?"TCP":"UDP",p->data.IO_type==0?"INOUT":"OUTIN",p->data.my_bind_port,inet_ntoa(t),p->data.des_bind_port);
				write(fd, sendbuff, strlen(sendbuff));
				fcntl(fd,F_UNLCK,0);
				fsync(fd);
			pid= fork();  
	 		if(pid== 0){     
				
				    
			if(p->data.Pro_type==PRO_TCP)
				tcpfunc(p->data.IO_type,p->data.my_bind_port,p->data.ip_addr,p->data.des_bind_port);
			else
				udpproxy(p->data.IO_type,p->data.my_bind_port,p->data.ip_addr,p->data.des_bind_port);
				exit(0);
   	 		}
			else if(pid < 0){
				exit(0);
			}
			if(pid>0){
				p->data.Enable = S_WORKING;
				p->data.pid = pid;
				p=p->next;
			}
		}
		else
			p = p->next;
	}
	close(fd);
}


void ListDeleteDisable(LinkList *L)
{

	struct flock lock;	//文件锁
	lock.l_type = F_WRLCK;
	lock.l_whence = 0;
	lock.l_start = 0; 
	lock.l_len = 0; 
	char time_s[30];
	char sendbuff[200];
	int fd ; //文件描述符
	fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND); 

	time_t rawtime;
	struct tm * ptm;
	struct in_addr t;

	  LinkList p = (*L)->next; 
    int i=1;
	ElemType *np;
    while(p ) {
	if(p->data.Enable==0){
		 if(p->data.pid!=S_WAITING){
			kill(p->data.pid, SIGKILL); 
			while(fcntl(fd, F_SETLK, &lock) < 0)
				{
					usleep(100);
					continue;
				}
					
			time ( &rawtime );

			ptm = gmtime(&rawtime);
			sprintf( time_s,"%4d-%02d-%02d %02d:%02d:%02d",ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
			t.s_addr = p->data.ip_addr;
			sprintf(sendbuff,"%s: 【服务关闭】%s %s 绑定端口号：%5d 目标IP地址: %15s 目标端口:%5d\n",time_s,p->data.Pro_type==0?"TCP":"UDP",p->data.IO_type==0?"INOUT":"OUTIN",p->data.my_bind_port,inet_ntoa(t),p->data.des_bind_port);
				write(fd, sendbuff, strlen(sendbuff));
				fcntl(fd,F_UNLCK,&lock);
				fsync(fd);
		 }
		 else
			printf("LinkList error\n");
		 M_ListDelete(L, i);
		 p = (*L)->next; 
		 i=1;
	}
	else	{
		i++;
		p = p->next;
	}
    }
	close(fd);
	
}


void SIGTERM_handler(int sig)
{
	Init_Config();
	ListDeleteDisable(&List);
	CreateNewConnect(List);
	Init_Config();
	CreateNewConnect(List);
	print(List);	//打印信息
}


int main()
{
	init_daemon();
	M_InitList(&List);	//初始化链表




	Init_Config();
	CreateNewConnect(List);
	print(List);
	signal(SIGCHLD,SIG_IGN);
	signal(SIGTERM,SIGTERM_handler);//设置kill 信号函数

	while(1)
		;
	
	M_DestroyList(&List);//销毁链表
	return 0;
}
