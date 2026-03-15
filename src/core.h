#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define BASE_TICK 5

#define RFIFOP(fd,pos) (session[fd]->rdata+session[fd]->rdata_pos+pos)
#define RFIFOB(fd,pos) (*(unsigned char*)(session[fd]->rdata+session[fd]->rdata_pos+pos))
#define RFIFOW(fd,pos) (*(unsigned short*)(session[fd]->rdata+session[fd]->rdata_pos+pos))
#define RFIFOL(fd,pos) (*(unsigned int*)(session[fd]->rdata+session[fd]->rdata_pos+pos))
#define RFIFOSKIP(fd,len) (session[fd]->rdata_pos+=len)
#define RFIFOREST(fd) (session[fd]->rdata_size-session[fd]->rdata_pos)
#define RFIFOFLUSH(fd) (memmove(session[fd]->rdata,RFIFOP(fd,0),RFIFOREST(fd)),session[fd]->rdata_size=RFIFOREST(fd),session[fd]->rdata_pos=0)
#define RFIFOSPACE(fd) (session[fd]->max_rdata-session[fd]->rdata_size)
#define RBUFP(p,pos) (((unsigned char*)(p))+pos)
#define RBUFB(p,pos) (*(unsigned char*)RBUFP(p,pos))
#define RBUFW(p,pos) (*(unsigned short*)RBUFP(p,pos))
#define RBUFL(p,pos) (*(unsigned int*)RBUFP(p,pos))

#define WFIFOSPACE(fd) (session[fd]->max_wdata-session[fd]->wdata_size)
#define WFIFOP(fd,pos) (session[fd]->wdata+session[fd]->wdata_size+pos)
#define WFIFOB(fd,pos) (*(unsigned char*)(session[fd]->wdata+session[fd]->wdata_size+pos))
#define WFIFOW(fd,pos) (*(unsigned short*)(session[fd]->wdata+session[fd]->wdata_size+pos))
#define WFIFOL(fd,pos) (*(unsigned int*)(session[fd]->wdata+session[fd]->wdata_size+pos))
#define WFIFOSET(fd,len) (session[fd]->wdata_size = (session[fd]->wdata_size+len+2048 < session[fd]->max_wdata) ? session[fd]->wdata_size+len : session[fd]->wdata_size)
#define WBUFP(p,pos) (((unsigned char*)(p))+pos)
#define WBUFB(p,pos) (*(unsigned char*)WBUFP(p,pos))
#define WBUFW(p,pos) (*(unsigned short*)WBUFP(p,pos))
#define WBUFL(p,pos) (*(unsigned int*)WBUFP(p,pos))

#define TIMER_ONCE_AUTODEL 0
#define TIMER_ONCE 1
#define TIMER_INTERVAL 2
extern struct TimerData {
  unsigned int tick;
  int (*func)(int,unsigned int,int,int);
  int id;
  int data;
  int type;
  int interval;
} **timer_data;

extern struct socket_data{
  int eof;
  unsigned char *rdata,*wdata;
  int max_rdata,max_wdata;
  int rdata_size,wdata_size;
  int rdata_pos;
  struct sockaddr_in client_addr;
  int (*func_recv)(int);
  int (*func_send)(int);
  int (*func_parse)(int);
  void* session_data;
} *session[FD_SETSIZE];

extern int rfifo_size,wfifo_size;
extern int (*default_func_parse)(int);
extern void (*term_func)(void);

int do_init(int,char**);
int make_listen_port(int);
int make_connection(long,int);
int delete_session(int);
unsigned int gettick(void);
int add_timer(unsigned int,int (*)(int,unsigned int,int,int),int,int);
int delete_timer(int,int (*)(int,unsigned int,int,int));
int add_basic_loop_func(int (*)(int));
int del_basic_loop_func(int);

