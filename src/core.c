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
#include <string.h>
#include <netinet/tcp.h>
#include "core.h"

int rfifo_size=32768;
int wfifo_size=32768;

int timer_data_max,timer_data_num;
fd_set readfds;
int fd_max;
void (*term_func)(void)=NULL;

unsigned int gettick(void)
{
  struct timeval tval;
  gettimeofday(&tval,NULL);
  return tval.tv_sec*1000+tval.tv_usec/1000;
}

void sig_proc(int sn)
{
  int i;
  switch(sn){
  case SIGINT:
  case SIGTERM:
    if(term_func)
      term_func();
    for(i=0;i<fd_max;i++){
      if(!session[i])
	continue;
      close(i);
    }
    exit(0);
    break;
  }
}

int recv_to_fifo(int fd)
{
  int len;

  //printf("recv_to_fifo : %d %d\n",fd,session[fd]->eof);
  if(session[fd]->eof)
    return -1;

  len=read(fd,session[fd]->rdata+session[fd]->rdata_size,RFIFOSPACE(fd));
  //{ int i; printf("recv %d : ",fd); for(i=0;i<len;i++){ printf("%02x ",RFIFOB(fd,i)); } printf("\n");}
  if(len>0){
    session[fd]->rdata_size+=len;
  } else if(len<=0){
    printf("set eof :%d\n",fd);
    session[fd]->eof=1;
  }
  return 0;
}

int send_from_fifo(int fd)
{
  int len;
  //printf("send_from_fifo : %d\n",fd);
  if(session[fd]->eof)
    return -1;
  len=write(fd,session[fd]->wdata,session[fd]->wdata_size);
  //{ int i; printf("send %d : ",fd);  for(i=0;i<len;i++){ printf("%02x ",session[fd]->wdata[i]); } printf("\n");}
  if(len>0){
    if(len<session[fd]->wdata_size){
      memmove(session[fd]->wdata,session[fd]->wdata+len,session[fd]->wdata_size-len);
      session[fd]->wdata_size-=len;
    } else {
      session[fd]->wdata_size=0;
    }
  } else {
    printf("set eof :%d\n",fd);
    session[fd]->eof=1;
  }

  return 0;
}

int null_parse(int fd)
{
  printf("null_parse : %d\n",fd);
  RFIFOSKIP(fd,RFIFOREST(fd));
  return 0;
}

int (*default_func_parse)(int) = null_parse;

int connect_client(int listen_fd)
{
  int fd;
  struct sockaddr_in client_address;
  int len;
  int result;

  //printf("connect_client : %d\n",listen_fd);

  len=sizeof(client_address);

  fd=accept(listen_fd,(struct sockaddr*)&client_address,&len);
  if(fd_max<=fd) fd_max=fd+1;
  result = fcntl(fd, F_SETFL, O_NONBLOCK);
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,NULL,0);
#ifdef SO_REUSEPORT
  setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,NULL,0);
#endif
  setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,NULL,0);

  if(fd==-1){
    perror("accept");
  } else {
    FD_SET(fd,&readfds);
  }
  result = fcntl(fd, F_SETFL, O_NONBLOCK);
  session[fd]=malloc(sizeof(*session[fd]));
  memset(session[fd],0,sizeof(*session[fd]));
  session[fd]->rdata=malloc(rfifo_size);
  session[fd]->wdata=malloc(wfifo_size);
  session[fd]->max_rdata=rfifo_size;
  session[fd]->max_wdata=wfifo_size;
  session[fd]->func_recv=recv_to_fifo;
  session[fd]->func_send=send_from_fifo;
  session[fd]->func_parse=default_func_parse;
  session[fd]->client_addr=client_address;

  //printf("new_session : %d %d\n",fd,session[fd]->eof);
  return fd;
}

int make_listen_port(int port)
{
  struct sockaddr_in server_address;
  int fd;
  int result;

  fd = socket( AF_INET, SOCK_STREAM, 0 );
  if(fd_max<=fd) fd_max=fd+1;
  result = fcntl(fd, F_SETFL, O_NONBLOCK);
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,NULL,0);
#ifdef SO_REUSEPORT
  setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,NULL,0);
#endif
  setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,NULL,0);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl( INADDR_ANY );
  server_address.sin_port = htons(port);

  result = bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
  if( result == -1 ) {
    perror("bind");
    exit(1);
  }
  result = listen( fd, 5 );
  if( result == -1 ) { /* error */
    perror("listen");
    exit(1);
  }

  FD_SET(fd, &readfds );
  session[fd]=malloc(sizeof(*session[fd]));
  memset(session[fd],0,sizeof(*session[fd]));
  session[fd]->func_recv=connect_client;

  return fd;
}

int make_connection(long ip,int port)
{
  struct sockaddr_in server_address;
  int fd;
  int result;

  fd = socket( AF_INET, SOCK_STREAM, 0 );
  if(fd_max<=fd) fd_max=fd+1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,NULL,0);
#ifdef SO_REUSEPORT
  setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,NULL,0);
#endif
  setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,NULL,0);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = ip;
  server_address.sin_port = htons(port);

  result = fcntl(fd, F_SETFL, O_NONBLOCK);
  result = connect(fd, (struct sockaddr *)(&server_address),sizeof(struct sockaddr_in));

  FD_SET(fd,&readfds);
  session[fd]=malloc(sizeof(*session[fd]));
  memset(session[fd],0,sizeof(*session[fd]));

  session[fd]->rdata=malloc(rfifo_size);
  session[fd]->wdata=malloc(wfifo_size);
  session[fd]->max_rdata=rfifo_size;
  session[fd]->max_wdata=wfifo_size;
  session[fd]->func_recv=recv_to_fifo;
  session[fd]->func_send=send_from_fifo;
  session[fd]->func_parse=default_func_parse;

  return fd;
}

int delete_session(int fd)
{
  if(fd<0 || fd>=FD_SETSIZE)
    return -1;
  FD_CLR(fd,&readfds);
  if(session[fd]){
    if(session[fd]->rdata)
      free(session[fd]->rdata);
    if(session[fd]->wdata)
      free(session[fd]->wdata);
    if(session[fd]->session_data)
      free(session[fd]->session_data);
    free(session[fd]);
  }
  session[fd]=NULL;
  //printf("delete_session:%d\n",fd);
  return 0;
}

int do_sendrecv(int next)
{
  fd_set rfd,wfd;
  struct timeval timeout;
  int ret,i;

  rfd=readfds;
  FD_ZERO(&wfd);
  for(i=0;i<fd_max;i++){
    if(!session[i] && FD_ISSET(i,&readfds)){
      printf("force clr fds %d\n",i);
      FD_CLR(i,&readfds);
      continue;
    }
    if(!session[i])
      continue;
    if(session[i]->wdata_size)
      FD_SET(i,&wfd);
  }
  timeout.tv_sec=next/1000;
  timeout.tv_usec=next%1000*1000;
  ret=select(fd_max,&rfd,&wfd,NULL,&timeout);
  if(ret<=0)
    return 0;
  for(i=0;i<fd_max;i++){
    if(!session[i])
      continue;
    if(FD_ISSET(i,&wfd)){
      //printf("write:%d\n",i);
      if(session[i]->func_send)
	session[i]->func_send(i);
    }
    if(FD_ISSET(i,&rfd)){
      //printf("read:%d\n",i);
      if(session[i]->func_recv)
	session[i]->func_recv(i);
    }
  }
  return 0;
}

int do_parsepacket(void)
{
  int i;
  for(i=0;i<fd_max;i++){
    if(!session[i])
      continue;
    if(session[i]->rdata_size==0 && session[i]->eof==0)
      continue;
    if(session[i]->func_parse){
      session[i]->func_parse(i);
      if(!session[i])
	continue;
    }
    RFIFOFLUSH(i);    
  }
  return 0;
}

int do_timer(unsigned int tick)
{ 
  int i,nextmin=1000;

  for(i=0;i<timer_data_num;i++){
    if(timer_data[i]==NULL)
      continue;
    if(((int)(timer_data[i]->tick-tick))>0)
      continue;
    //printf("@%08x : %d %d %d %x()\n",tick,i,timer_data[i]->id,timer_data[i]->data,timer_data[i]->func);
    if(timer_data[i]->func)
      timer_data[i]->func(i,tick,timer_data[i]->id,timer_data[i]->data);
  }
  for(i=0;i<timer_data_num;i++){
    if(timer_data[i]==NULL || ((int)(timer_data[i]->tick-tick))>0)
      continue;
    switch(timer_data[i]->type){
    case TIMER_ONCE_AUTODEL:
      free(timer_data[i]);
      timer_data[i]=NULL;
      break;
    case TIMER_INTERVAL:
      timer_data[i]->tick+=timer_data[i]->interval;
      break;
    }
  }
  for(i=0;i<timer_data_num;i++){
    if(timer_data[i]==NULL)
      continue;
    if((int)(timer_data[i]->tick-tick)<nextmin){
      nextmin=timer_data[i]->tick-tick;
    }
  }
  if(nextmin<10)
    nextmin=10;
  return nextmin;
}

int add_timer(unsigned int tick,int (*func)(int,unsigned int,int,int),int id,int data)
{ 
  struct TimerData *td;
  int i;

  td=malloc(sizeof(struct TimerData));
  if(td==NULL){
    fprintf(stderr,"add_timer: malloc\n");
    exit(1);
  }
  for(i=1;i<timer_data_num;i++)
    if(timer_data[i]==NULL)
      break;
  if(i>=timer_data_num && i>=timer_data_max){
    if(timer_data_max==0){
      timer_data_max=256;
      timer_data=malloc(sizeof(struct TimerData*)*timer_data_max);
      timer_data[0]=NULL;
    } else {
      timer_data_max+=256;
      timer_data=realloc(timer_data,sizeof(struct TimerData*)*timer_data_max);
    }
  }
  td->tick=tick;
  td->func=func;
  td->id=id;
  td->data=data;
  td->type=TIMER_ONCE_AUTODEL;
  td->interval=1000;
  timer_data[i]=td;
  if(i>=timer_data_num)
    timer_data_num=i+1;
  return i;
}

int delete_timer(int id,int (*func)(int,unsigned int,int,int))
{
  //printf("delete_timer %d\n",id);
  if(id<=0 || id>=timer_data_num || timer_data[id]==NULL)
    return -1;
  //printf("free %d\n",id);
  if(timer_data[id]->func != func){
 //   printf("%08x != %08x\n",timer_data[id]->func,func);
    return -2;
  }
  free(timer_data[id]);
  timer_data[id]=NULL;
  return 0;
}

#define MAX_BLF 16

int (*basic_loop_func[MAX_BLF])(int);
int basic_loop_func_num=0;

int add_basic_loop_func(int (*blf)(int))
{
  if(basic_loop_func_num>=MAX_BLF)
    return -1;
  basic_loop_func[basic_loop_func_num++]=blf;
  return basic_loop_func_num-1;
}

int del_basic_loop_func(int id)
{
  int i;
  if(id<0 || id>=basic_loop_func_num)
    return -1;
  if(id!=basic_loop_func_num-1){
    for(i=id;i<basic_loop_func_num-1;i++)
      basic_loop_func[i]=basic_loop_func[i+1];
  }
  basic_loop_func_num--;
  basic_loop_func[basic_loop_func_num]=NULL;
  return 0;
}

int main(int argc,char **argv)
{
  int tick;
  int i,next;

  FD_ZERO(&readfds);
  signal(SIGPIPE,SIG_IGN);
  signal(SIGTERM,sig_proc);
  signal(SIGINT,sig_proc);
  do_init(argc,argv);
  tick=gettick();
  while(1){
    next=do_timer(tick);
    do_sendrecv(next);
    do_parsepacket();
    for(i=0;i<basic_loop_func_num;i++)
      if(basic_loop_func[i])
	basic_loop_func[i](tick);
    tick=gettick();
  }
  return 0;
}
