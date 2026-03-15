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
#include <arpa/inet.h>

#include "core.h"
#include "mmo.h"

int account_id_count = 1000000;
int server_num;

struct mmo_char_server server[MAX_SERVERS];
int server_fd[MAX_SERVERS];

#define AUTH_FIFO_SIZE 256
struct {
  int account_id,login_id1,login_id2;
  int delflag;
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos=0;
struct {
  int account_id,sex;
  char userid[24],pass[24],lastlogin[24];
  int logincount;
} *auth_dat;
int auth_num=0,auth_max=0;

int mmo_auth_init(void)
{
  FILE *fp;
  int i,account_id,logincount;
  char line[1024],userid[24],pass[24],lastlogin[24],sex;
  fp=fopen("data/accounts.txt","r");
  auth_dat=malloc(sizeof(auth_dat[0])*256);
  auth_max=256;
  if(fp==NULL)
    return 0;
  while(fgets(line,1023,fp)!=NULL){
    i=sscanf(line,"%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d",
	     &account_id,userid,pass,lastlogin,&sex,&logincount);
    if(i>=5){
      if(auth_num>=auth_max){
	auth_max+=256;
	auth_dat=realloc(auth_dat,sizeof(auth_dat[0])*auth_max);
      }
      auth_dat[auth_num].account_id=account_id;
      strncpy(auth_dat[auth_num].userid,userid,24);
      strncpy(auth_dat[auth_num].pass,pass,24);
      strncpy(auth_dat[auth_num].lastlogin,lastlogin,24);
      auth_dat[auth_num].sex = sex == 'S' ? 2 : sex=='M';
      if(i>=6)
	auth_dat[auth_num].logincount=logincount;
      else
	auth_dat[auth_num].logincount=1;
      auth_num++;
      if(account_id>=account_id_count)
	account_id_count=account_id+1;
    }
  }
  fclose(fp);

  return 0;
}

void mmo_auth_sync(void)
{
  FILE *fp;
  int i;
  fp=fopen("data/accounts.txt","w");
  if(fp==NULL)
    return;
  for(i=0;i<auth_num;i++){
    fprintf(fp,"%d\t%s\t%s\t%s\t%c\t%d\n",auth_dat[i].account_id,
	    auth_dat[i].userid,auth_dat[i].pass,auth_dat[i].lastlogin,
	    auth_dat[i].sex==2 ? 'S' : (auth_dat[i].sex ? 'M' : 'F'),
	    auth_dat[i].logincount);
  }
  fclose(fp);
}

int mmo_auth( struct mmo_account* account )
{
  int i;
  struct timeval tv;
  char tmpstr[256];
  FILE *logfp;
  int len,newaccount=0;

  len=strlen(account->userid)-2;
  if(account->userid[len]=='_' && (account->userid[len+1]=='F' || account->userid[len+1]=='M')){
    newaccount=1;
    account->userid[len]=0;
  }
  gettimeofday(&tv,NULL);
  strftime(tmpstr,24,"%Y-%m-%d %H:%M:%S",localtime(&(tv.tv_sec)));
  sprintf(tmpstr+19,".%03d",(int)tv.tv_usec/1000);

  for(i=0;i<auth_num;i++){
    if(strcmp(account->userid,auth_dat[i].userid)==0)
      break;
  }
  if(i!=auth_num){
    if(strcmp(account->passwd,auth_dat[i].pass) || newaccount){
      logfp=fopen("logs/login.log","a");
      if(logfp){
	fprintf(logfp,"auth failed pass error %s %s %s %d\n",tmpstr,account->userid,account->passwd,newaccount);
	fclose(logfp);
      }
      return 1;
    }
    logfp=fopen("logs/login.log","a");
    if(logfp){
      fprintf(logfp,"auth ok %s %s %s %d\n",tmpstr,account->userid,account->passwd,newaccount);
      fclose(logfp);
    }
  } else {
    if(newaccount==0){
      logfp=fopen("logs/login.log","a");
      if(logfp){
	fprintf(logfp,"auth failed no account %s %s %s %d\n",tmpstr,account->userid,account->passwd,newaccount);
	fclose(logfp);
      }
      return 0;
    }
    logfp=fopen("logs/login.log","a");
    if(logfp){
      fprintf(logfp,"auth new %s %s %s %d\n",tmpstr,account->userid,account->passwd,newaccount);
      fclose(logfp);
    }
    if(auth_num>=auth_max){
      auth_max+=256;
      auth_dat=realloc(auth_dat,sizeof(auth_dat[0])*auth_max);
    }
    auth_dat[i].account_id=account_id_count++;
    strncpy(auth_dat[i].userid,account->userid,24);
    strncpy(auth_dat[i].pass,account->passwd,24);
    auth_dat[i].sex=account->userid[len+1]=='M';
    auth_dat[i].logincount=0;
    auth_num++;
  }
  account->account_id = auth_dat[i].account_id;
  account->login_id1 = rand();
  account->login_id2 = rand();
  memcpy(account->lastlogin,auth_dat[i].lastlogin,24);
  memcpy(auth_dat[i].lastlogin,tmpstr,24);
  account->sex = auth_dat[i].sex;
  auth_dat[i].logincount++;

  mmo_auth_sync();
  return 100;
}

int parse_fromchar(int fd)
{
  int i,id;

  for(id=0;id<MAX_SERVERS;id++)
    if(server_fd[id]==fd)
      break;
  if(id==MAX_SERVERS)
    session[fd]->eof=1;
  if(session[fd]->eof){
    for(i=0;i<MAX_SERVERS;i++)
      if(server_fd[i]==fd)
	server_fd[i]=-1;
    close(fd);
    delete_session(fd);
    return 0;
  }
  while(RFIFOREST(fd)>=2){
    switch(RFIFOW(fd,0)){
    case 0x2712:
      if(RFIFOREST(fd)<14)
	return 0;
      for(i=0;i<AUTH_FIFO_SIZE;i++){
	if(auth_fifo[i].account_id==RFIFOL(fd,2) &&
	   auth_fifo[i].login_id1==RFIFOL(fd,6) &&
	   auth_fifo[i].login_id2==RFIFOL(fd,10) &&
	   !auth_fifo[i].delflag){
	  auth_fifo[i].delflag=1;
	  break;
	}
      }
      WFIFOW(fd,0)=0x2713;
      WFIFOL(fd,2)=RFIFOL(fd,2);
      if(i!=AUTH_FIFO_SIZE){
	WFIFOB(fd,6)=0;
      } else {
	WFIFOB(fd,6)=1;
      }
      WFIFOSET(fd,7);
      RFIFOSKIP(fd,14);
      break;
    case 0x2714:
      //printf("set users %s : %d\n",server[id].name,RFIFOL(fd,2));
      server[id].users=RFIFOL(fd,2);
      RFIFOSKIP(fd,6);
      break;
    case 0x3002:
    	mmo_auth_init();
	    printf("GMˆß‘••ÔM€”õŠ®—¹I");
	    RFIFOSKIP(fd,2);
	   	break;
    default:
      close(fd);
      session[fd]->eof=1;
      return 0;
    }
  }
  return 0;
}

int parse_login(int fd)
{
  struct mmo_account account;
  int result,i;

  if(session[fd]->eof){
    for(i=0;i<MAX_SERVERS;i++)
      if(server_fd[i]==fd)
	server_fd[i]=-1;
    close(fd);
    delete_session(fd);
    return 0;
  }
  printf("parse_login : %d %d %d\n",fd,RFIFOREST(fd),RFIFOW(fd,0));
  while(RFIFOREST(fd)>=2){
    switch(RFIFOW(fd,0)){
    case 0x64:
      if(RFIFOREST(fd)<55)
	return 0;
      {
	FILE *logfp=fopen("logs/login.log","a");
	if(logfp){
	  unsigned char *p=(unsigned char *)&session[fd]->client_addr.sin_addr;
	  fprintf(logfp,"client connection request %s from %d.%d.%d.%d\n",
		  RFIFOP(fd,6),p[0],p[1],p[2],p[3]);
	  fclose(logfp);
	}
      }
      account.userid = RFIFOP(fd,6);
      account.passwd = RFIFOP(fd,30);
      result=mmo_auth(&account);
      if(result==100){
	server_num=0;
	for(i=0;i<MAX_SERVERS;i++){
	  if(server_fd[i]>=0){
	    WFIFOL(fd,47+server_num*32) = server[i].ip;
	    WFIFOW(fd,47+server_num*32+4) = server[i].port;
	    memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20 );
	    WFIFOW(fd,47+server_num*32+26) = server[i].users;
	    WFIFOW(fd,47+server_num*32+28) = 0; // maintenance
	    WFIFOW(fd,47+server_num*32+30) = 0; // new
	    server_num++;
	  }
	}
	WFIFOW(fd,0)=0x69;
	WFIFOW(fd,2)=47+32*server_num;
	WFIFOL(fd,4)=account.login_id1;
	WFIFOL(fd,8)=account.account_id;
	WFIFOL(fd,12)=account.login_id2;
	WFIFOL(fd,16)=0;
	memcpy(WFIFOP(fd,20),account.lastlogin,24);
	WFIFOB(fd,46)=account.sex;
	WFIFOSET(fd,47+32*server_num);
	if(auth_fifo_pos>=AUTH_FIFO_SIZE){
	  auth_fifo_pos=0;
	}
	auth_fifo[auth_fifo_pos].account_id=account.account_id;
	auth_fifo[auth_fifo_pos].login_id1=account.login_id1;
	auth_fifo[auth_fifo_pos].login_id2=account.login_id2;
	auth_fifo[auth_fifo_pos].delflag=0;
	auth_fifo_pos++;
      } else {
	WFIFOW(fd,0)=0x6a;
	WFIFOB(fd,2)=result;
	WFIFOSET(fd,23);
      }
      RFIFOSKIP(fd,55);
      break;
    case 0x2710:
      if(RFIFOREST(fd)<76)
	return 0;
      {
	FILE *logfp=fopen("logs/login.log","a");
	if(logfp){
	  unsigned char *p=(unsigned char *)&session[fd]->client_addr.sin_addr;
	  fprintf(logfp,"server connection request %s @ %d.%d.%d.%d:%d (%d.%d.%d.%d)\n",
		  RFIFOP(fd,60),RFIFOB(fd,54),RFIFOB(fd,55),RFIFOB(fd,56),RFIFOB(fd,57),RFIFOW(fd,58),
		  p[0],p[1],p[2],p[3]);
	  fclose(logfp);
	}
      }
      account.userid = RFIFOP(fd,2);
      account.passwd = RFIFOP(fd,26);
      result = mmo_auth(&account);
      if(result == 100 && account.sex==2 && account.account_id<MAX_SERVERS && server_fd[account.account_id]<0){
	memcpy(server[account.account_id].name,RFIFOP(fd,60),16);
	server[account.account_id].ip=RFIFOL(fd,54);
	server[account.account_id].port=RFIFOW(fd,58);
	server[account.account_id].users=0;
	server_fd[account.account_id]=fd;
	WFIFOW(fd,0)=0x2711;
	WFIFOB(fd,2)=0;
	WFIFOSET(fd,3);
	session[fd]->func_parse=parse_fromchar;
      } else {
	WFIFOW(fd,0)=0x2711;
	WFIFOB(fd,2)=3;
	WFIFOSET(fd,3);
      }
      RFIFOSKIP(fd,76);
      return 0;
    default:
      close(fd);
      session[fd]->eof=1;
      return 0;
    }
  }
  return 0;
}

int do_init(int argc,char **argv)
{
  int i;
  printf("YARE 0.01 Login-Server\n(c) Project YARE\nwww.project-yare.net\n\n");
  for(i=0;i<AUTH_FIFO_SIZE;i++){
    auth_fifo[i].delflag=1;
  }
  for(i=0;i<MAX_SERVERS;i++){
    server_fd[i]=-1;
  }
  default_func_parse=parse_login;
  make_listen_port(6900);
  mmo_auth_init();
  term_func=mmo_auth_sync;
printf("Login-Server is online.\n");
  return 0;
}
