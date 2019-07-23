#include <stdlib.h>
#include <stdio.h>
#include "LinkList.h"



/* ??????????? */
Status M_InitList(LinkList *L)
{
    *L = (LNode *)malloc(sizeof(LNode));
    if (*L==NULL)
	exit(OVERFLOW);

    (*L)->next = NULL;
    return OK;
}

/* ????????????????????? */
Status M_DestroyList(LinkList *L)
{
    LinkList q, p = *L;
    while(p) {   
		q=p->next; 
		free(p);
		p=q;
		}
    *L=NULL;	
    return OK;
}

/* ??????????????????? */
Status M_ClearList(LinkList *L)
{
    LinkList q, p = (*L)->next;
    while(p) {
		q = p->next;   
		free(p);
		p = q;
		}
    (*L)->next = NULL;
    return OK;
}

/* ?§Ø???????? */
Status M_ListEmpty(LinkList L)
{
    if (L->next==NULL)
		return TRUE;
    else
		return FALSE;
}

/* ???????? */
int M_ListLength(LinkList L)
{
    LinkList p = L->next; 
    int len=0;
    while(p) {
	p = p->next;
	len++;
	}
    return len;
}

Status M_DisableList(LinkList L)
{
    LinkList p = L->next; 
    while(p) {
	p->data.Enable=S_WAITING;
	p = p->next;
	}
    return OK;
}

Status M_EnableList(LinkList L,int n)
{
    LinkList p = L->next; 
    int i=1;
    while(p &&i<n) {
	p = p->next;
	i++;
	}
	if(!p ||i>n)
		return ERROR;
    if(p->data.Enable==0)
	    p->data.Enable=S_WORKING;
    return OK;
}

/*
Status M_ListDeleteDisable(LinkList *L)
{
    LinkList p = (*L)->next; 
    int i=1;
    while(p ) {
	if(p->data.Enable==0){
		 ListDelete(L, i);
		 p = (*L)->next; 
		 i=1;
	}
	else	{
		i++;
		p = p->next;
	}
    }
   
    return OK;
}
*/

/* ????§Ö?i????? */
Status M_GetElem(LinkList L, int i, ElemType *e)
{
    LinkList p = L->next;
    int    pos = 1;	
    while(p!=NULL && pos<i) {
    	p=p->next;
    	pos++;
		}

    if (!p || pos>i)
    	return ERROR;
    e->des_bind_port = p->data.des_bind_port;
    e->IO_type =p->data.IO_type;
    e->ip_addr=p->data.ip_addr;
    e->my_bind_port=p->data.my_bind_port;
    e->pid=p->data.pid;
    e->Pro_type=p->data.Pro_type;
    e->Enable=p->data.Enable;
    return OK;
}

/* ??????????????????? */
int M_LocateElem(LinkList L, ElemType e)
{
    LinkList p = L->next;
    int    pos = 1;	

    while(p  ) {
   /*
    	if(p->data.IO_type==e.IO_type && p->data.Pro_type==e.Pro_type&&p->data.des_bind_port==e.des_bind_port	\
			&&p->data.ip_addr== e.ip_addr &&p->data.my_bind_port==e.my_bind_port)
    		break;
    		*/
    	if(p->data.my_bind_port==e.my_bind_port)
    		break;	
		p=p->next;
		pos++;
	}
    return p ? pos:0;
}

int M_LocateSameElem(LinkList L, ElemType e)
{
    LinkList p = L->next;
    int    pos = 1;	

    while(p  ) {
   
    	if(p->data.IO_type==e.IO_type && p->data.Pro_type==e.Pro_type&&p->data.des_bind_port==e.des_bind_port	\
			&&p->data.ip_addr== e.ip_addr &&p->data.my_bind_port==e.my_bind_port)
    		break;
		p=p->next;
		pos++;
	}
    return p ? pos:0;
}


/* ?????¦Ë??????????????? */
Status M_ListInsert(LinkList *L, int i, ElemType e)
{
    LinkList s, p = *L;	
    int      pos  = 0;

    while(p && pos<i-1) {
		p=p->next;
		pos++;
		}

    if (p==NULL || pos>i-1)  
		return ERROR;

    s = (LinkList)malloc(sizeof(LNode));
    if (s==NULL)
		return OVERFLOW;

    s->data.Pro_type = e.Pro_type; 
    s->data.IO_type=e.IO_type;
    s->data.des_bind_port=e.des_bind_port;
    s->data.ip_addr=e.ip_addr;
    s->data.my_bind_port=e.my_bind_port;
    s->data.pid=e.pid;
    s->data.Enable =e.Enable;
    s->next = p->next;
    p->next = s;
    return OK;
}

/* ??????¦Ë?????????????????????????e?§Ù??? */
Status M_ListDelete(LinkList *L, int i)
{
    LinkList q, p = *L;	
    int      pos  = 0;
    while(p->next && pos<i-1) {
		p=p->next;
		pos++;
		}

    if (p->next==NULL || pos>i-1)	
		return ERROR;
	
    q = p->next;      
    p->next = q->next; 
    free(q);              
    return OK;
}

