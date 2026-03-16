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
#include "char2.h"

struct mmo_map_server server[MAX_SERVERS];
int server_fd[MAX_SERVERS];

int login_fd;
char userid[24];
char passwd[24];
char server_name[16];
char login_ip_str[16];
int login_port;
char char_ip_str[16];
int char_ip;
int char_port;
char char_txt[256]="data/players.txt";

#define CHAR_STATE_WAITAUTH 0
#define CHAR_STATE_AUTHOK 1
struct char_session_data{
  int state;
  int account_id,login_id1,login_id2,sex;
  int found_char[3];
};

#define AUTH_FIFO_SIZE 256
struct {
  int account_id,char_id,login_id1,char_pos,delflag;
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos=0;

int char_id_count=100000;
struct mmo_charstatus *char_dat;
int char_num,char_max;

int mmo_char_tostr(char *str,struct mmo_charstatus *p)
{
  int i;
  sprintf(str,"%d\t%d,%d\t%s\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
	  "\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
	  "\t%s,%d,%d\t%s,%d,%d",
	  p->char_id,p->account_id,p->char_num,p->name, //
	  p->class,p->base_level,p->job_level,
	  p->base_exp,p->job_exp,p->zeny,
	  p->hp,p->max_hp,p->sp,p->max_sp,
	  p->str,p->agi,p->vit,p->int_,p->dex,p->luk,
	  p->status_point,p->skill_point,
	  p->option,p->karma,p->manner,	//
	  p->party_id,p->guild_id,
	  p->hair,p->hair_color,p->clothes_color,
	  p->weapon,p->sheild,p->head_top,p->head_mid,p->head_bottom,
	  p->last_point.map,p->last_point.x,p->last_point.y, //
	  p->save_point.map,p->save_point.x,p->save_point.y
	  );
  strcat(str,"\t");
  for(i=0;i<3;i++)
    if(p->memo_point[i].map[0]){
      sprintf(str+strlen(str),"%s,%d,%d",p->memo_point[i].map,p->memo_point[i].x,p->memo_point[i].y);
    }      
  strcat(str,"\t");
  for(i=0;i<MAX_INVENTORY;i++)
    if(p->inventory[i].nameid){
      sprintf(str+strlen(str),"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
	      p->inventory[i].id,p->inventory[i].nameid,p->inventory[i].amount,p->inventory[i].equip,
	      p->inventory[i].identify,p->inventory[i].refine,p->inventory[i].attribute,
	      p->inventory[i].card[0],p->inventory[i].card[1],p->inventory[i].card[2],p->inventory[i].card[3]);
    }      
  strcat(str,"\t");
  for(i=0;i<MAX_CART;i++)
    if(p->cart[i].nameid){
      sprintf(str+strlen(str),"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
	      p->cart[i].id,p->cart[i].nameid,p->cart[i].amount,p->cart[i].equip,
	      p->cart[i].identify,p->cart[i].refine,p->cart[i].attribute,
	      p->cart[i].card[0],p->cart[i].card[1],p->cart[i].card[2],p->cart[i].card[3]);
    }      
  strcat(str,"\t");
  for(i=0;i<MAX_SKILL;i++)
    if(p->skill[i].id){
      sprintf(str+strlen(str),"%d,%d ",p->skill[i].id,p->skill[i].lv);
    }      
  strcat(str,"\t");
  return 0;
}

int mmo_char_fromstr(char *str,struct mmo_charstatus *p)
{
  int tmp_int[256];
  int set,next,len,i;

  set=sscanf(str,"%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
	   "\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
	   "\t%[^,],%d,%d\t%[^,],%d,%d%n",
	   &tmp_int[0],&tmp_int[1],&tmp_int[2],p->name, //
	   &tmp_int[3],&tmp_int[4],&tmp_int[5],
	   &tmp_int[6],&tmp_int[7],&tmp_int[8],
	   &tmp_int[9],&tmp_int[10],&tmp_int[11],&tmp_int[12],
	   &tmp_int[13],&tmp_int[14],&tmp_int[15],&tmp_int[16],&tmp_int[17],&tmp_int[18],
	   &tmp_int[19],&tmp_int[20],
	   &tmp_int[21],&tmp_int[22],&tmp_int[23], //
	   &tmp_int[24],&tmp_int[25],
	   &tmp_int[26],&tmp_int[27],&tmp_int[28],
	   &tmp_int[29],&tmp_int[30],&tmp_int[31],&tmp_int[32],&tmp_int[33],
	   p->last_point.map,&tmp_int[34],&tmp_int[35], //
	   p->save_point.map,&tmp_int[36],&tmp_int[37],&next
	 );
  p->char_id=tmp_int[0];
  p->account_id=tmp_int[1];
  p->char_num=tmp_int[2];
  p->class=tmp_int[3];
  p->base_level=tmp_int[4];
  p->job_level=tmp_int[5];
  p->base_exp=tmp_int[6];
  p->job_exp=tmp_int[7];
  p->zeny=tmp_int[8];
  p->hp=tmp_int[9];
  p->max_hp=tmp_int[10];
  p->sp=tmp_int[11];
  p->max_sp=tmp_int[12];
  p->str=tmp_int[13];
  p->agi=tmp_int[14];
  p->vit=tmp_int[15];
  p->int_=tmp_int[16];
  p->dex=tmp_int[17];
  p->luk=tmp_int[18];
  p->status_point=tmp_int[19];
  p->skill_point=tmp_int[20];
  p->option=tmp_int[21];
  p->karma=tmp_int[22];
  p->manner=tmp_int[23];
  p->party_id=tmp_int[24];
  p->guild_id=tmp_int[25];
  p->hair=tmp_int[26];
  p->hair_color=tmp_int[27];
  p->clothes_color=tmp_int[28];
  p->weapon=tmp_int[29];
  p->sheild=tmp_int[30];
  p->head_top=tmp_int[31];
  p->head_mid=tmp_int[32];
  p->head_bottom=tmp_int[33];
  p->last_point.x=tmp_int[34];
  p->last_point.y=tmp_int[35];
  p->save_point.x=tmp_int[36];
  p->save_point.y=tmp_int[37];
  if(set!=41)
    return 0;
  if(str[next]=='\n')
    return 1;	// 布疤高垂
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%[^,],%d,%d%n",p->memo_point[i].map,&tmp_int[0],&tmp_int[1],&len);
    if(set!=3) 
      return 0;
    p->memo_point[i].x=tmp_int[0];
    p->memo_point[i].y=tmp_int[1];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
	       &tmp_int[0],&tmp_int[1],&tmp_int[2],&tmp_int[3],
	       &tmp_int[4],&tmp_int[5],&tmp_int[6],
	       &tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10],&len);
    if(set!=11)
      return 0;
    p->inventory[i].id=tmp_int[0];
    p->inventory[i].nameid=tmp_int[1];
    p->inventory[i].amount=tmp_int[2];
    p->inventory[i].equip=tmp_int[3];
    p->inventory[i].identify=tmp_int[4];
    p->inventory[i].refine=tmp_int[5];
    p->inventory[i].attribute=tmp_int[6];
    p->inventory[i].card[0]=tmp_int[7];
    p->inventory[i].card[1]=tmp_int[8];
    p->inventory[i].card[2]=tmp_int[9];
    p->inventory[i].card[3]=tmp_int[10];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
	       &tmp_int[0],&tmp_int[1],&tmp_int[2],&tmp_int[3],
	       &tmp_int[4],&tmp_int[5],&tmp_int[6],
	       &tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10],&len);
    if(set!=11)
      return 0;
    p->cart[i].id=tmp_int[0];
    p->cart[i].nameid=tmp_int[1];
    p->cart[i].amount=tmp_int[2];
    p->cart[i].equip=tmp_int[3];
    p->cart[i].identify=tmp_int[4];
    p->cart[i].refine=tmp_int[5];
    p->cart[i].attribute=tmp_int[6];
    p->cart[i].card[0]=tmp_int[7];
    p->cart[i].card[1]=tmp_int[8];
    p->cart[i].card[2]=tmp_int[9];
    p->cart[i].card[3]=tmp_int[10];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%d,%d%n",
	       &tmp_int[0],&tmp_int[1],&len);
    if(set!=2)
      return 0;
	p->skill[tmp_int[0]-1].id=tmp_int[0];
	p->skill[tmp_int[0]-1].lv=tmp_int[1];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  return 1;
}

int mmo_char_init(void)
{
  char line[65536];
  int ret;
  FILE *fp;
	char_num=0;
  fp=fopen(char_txt,"r");
  char_dat=malloc(sizeof(char_dat[0])*256);
  char_max=256;
  if(fp==NULL)
    return 0;
  while(fgets(line,65535,fp)){
    if(char_num>=char_max){
      char_max+=256;
      char_dat=realloc(char_dat,sizeof(char_dat[0])*char_max);
    }
    memset(&char_dat[char_num],0,sizeof(char_dat[0]));
    ret=mmo_char_fromstr(line,&char_dat[char_num]);
    if(ret){
      if(char_dat[char_num].char_id>=char_id_count)
	char_id_count=char_dat[char_num].char_id+1;
      char_num++;
    }
  }
  fclose(fp);
  return 0;
}

void mmo_char_sync(void)
{
  char line[65536];
  int i;
  FILE *fp;
  fp=fopen(char_txt,"w");
  if(fp==NULL)
    return;
  for(i=0;i<char_num;i++){
    mmo_char_tostr(line,&char_dat[i]);
    fprintf(fp,"%s\n",line);
  }
  fclose(fp);
}

int make_new_char(int fd,unsigned char *dat)
{
  int i;
  struct char_session_data *sd;
  FILE *logfp;

  if(dat[24]+dat[25]+dat[26]+dat[27]+dat[28]+dat[29]>5*6 ||
     dat[30]>=3 ||
     dat[33]==0 || dat[33]>=20 ||
     dat[31]>=9){
    logfp=fopen("logs/char.log","a");
    if(logfp){
      fprintf(logfp,"make new char error %d-%d %s %d,%d,%d,%d,%d,%d %d,%d\n",
	      fd,dat[30],dat,dat[24],dat[25],dat[26],dat[27],dat[28],dat[29],dat[33],dat[31]);
      fclose(logfp);
    }
    return -1;
  }
  logfp=fopen("logs/char.log","a");
  if(logfp){
    fprintf(logfp,"make new char %d-%d %s\n",fd,dat[30],dat);
    fclose(logfp);
  }
  sd=session[fd]->session_data;

  for(i=0;i<char_num;i++){
    if(strcmp(char_dat[i].name,(char*)dat)==0 || (char_dat[i].account_id==sd->account_id && char_dat[i].char_num==dat[30]))
      break;
  }
  if(i!=char_num)
    return -1;
  if(char_num>=char_max){
    char_max+=256;
    char_dat=realloc(char_dat,sizeof(char_dat[0])*char_max);
  }
  char_dat[i].char_id=char_id_count++;
  char_dat[i].account_id=sd->account_id;
  char_dat[i].char_num=dat[30];
  strcpy(char_dat[i].name,(char*)dat);
  char_dat[i].class=0;
  char_dat[i].base_level=1;
  char_dat[i].job_level=1;
  char_dat[i].base_exp=0;
  char_dat[i].job_exp=0;
  char_dat[i].zeny=500;
  char_dat[i].hp=40;
  char_dat[i].max_hp=40;
  char_dat[i].sp=2;
  char_dat[i].max_sp=2;
  char_dat[i].str=dat[24];
  char_dat[i].agi=dat[25];
  char_dat[i].vit=dat[26];
  char_dat[i].int_=dat[27];
  char_dat[i].dex=dat[28];
  char_dat[i].luk=dat[29];
  char_dat[i].status_point=0;
  char_dat[i].skill_point=0;
  char_dat[i].option=0;
  char_dat[i].karma=0;
  char_dat[i].manner=0;
  char_dat[i].party_id=0;
  char_dat[i].guild_id=0;
  char_dat[i].hair=dat[33];
  char_dat[i].hair_color=dat[31];
  char_dat[i].clothes_color=0;
  char_dat[i].weapon=0;
  char_dat[i].sheild=0;
  char_dat[i].head_top=0;
  char_dat[i].head_mid=0;
  char_dat[i].head_bottom=0;
  strcpy(char_dat[i].last_point.map,"new_1-1.gat");
  char_dat[i].last_point.x=53;
  char_dat[i].last_point.y=111;
  strcpy(char_dat[i].save_point.map,"new_1-1.gat");
  char_dat[i].save_point.x=53;
  char_dat[i].save_point.y=111;
  char_num++;

  mmo_char_sync();
  return i;
}

int mmo_char_send006b(int fd,struct char_session_data *sd)
{
  int i,j,found_num;
#ifdef NEW_006b
  inf offset=24;
#else
  int offset=4;
#endif

  sd->state=CHAR_STATE_AUTHOK;
  /* 鲁いてキャラデ〖タを积ってきて流E栅堡丒*/
  for(i=found_num=0;i<char_num;i++){
    if(char_dat[i].account_id==sd->account_id){
      sd->found_char[found_num]=i;
      found_num++;
      if(found_num==3)
	break;
    }
  }
  for(i=found_num;i<3;i++)
    sd->found_char[i]=-1;

  WFIFOW(fd,0)=0x6b;
  WFIFOW(fd,2)=offset+found_num*106;
  for( i = 0; i < found_num; i++ ) {

    j=sd->found_char[i];

    memset(WFIFOP(fd,offset+(i*106)),0x00,106);

    WFIFOL(fd,offset+(i*106)) = char_dat[j].char_id;
    WFIFOL(fd,offset+(i*106)+4) = char_dat[j].base_exp;
    WFIFOL(fd,offset+(i*106)+8) = char_dat[j].zeny;
    WFIFOL(fd,offset+(i*106)+12) = char_dat[j].job_exp;
    WFIFOL(fd,offset+(i*106)+16) = char_dat[j].job_level;

    WFIFOL(fd,offset+(i*106)+20) = 0;
    WFIFOL(fd,offset+(i*106)+24) = 0;
    WFIFOL(fd,offset+(i*106)+28) = char_dat[j].option;

    WFIFOL(fd,offset+(i*106)+32) = char_dat[j].karma;
    WFIFOL(fd,offset+(i*106)+36) = char_dat[j].manner;

    WFIFOW(fd,offset+(i*106)+40) = char_dat[j].status_point;
    WFIFOW(fd,offset+(i*106)+42) = char_dat[j].hp;
    WFIFOW(fd,offset+(i*106)+44) = char_dat[j].max_hp;
    WFIFOW(fd,offset+(i*106)+46) = char_dat[j].sp;
    WFIFOW(fd,offset+(i*106)+48) = char_dat[j].max_sp;
    WFIFOW(fd,offset+(i*106)+50) = DEFAULT_WALK_SPEED; // char_dat[j].speed;
    WFIFOW(fd,offset+(i*106)+52) = char_dat[j].class;
    WFIFOW(fd,offset+(i*106)+54) = char_dat[j].hair;
    WFIFOW(fd,offset+(i*106)+56) = char_dat[j].weapon;//曄峏偟傑偟偨
    WFIFOW(fd,offset+(i*106)+58) = char_dat[j].base_level;
    WFIFOW(fd,offset+(i*106)+60) = char_dat[j].skill_point;
    WFIFOW(fd,offset+(i*106)+62) = char_dat[j].head_bottom;//曄峏偟傑偟偨
    WFIFOW(fd,offset+(i*106)+64) = char_dat[j].sheild;
    WFIFOW(fd,offset+(i*106)+66) = char_dat[j].head_top;
    WFIFOW(fd,offset+(i*106)+68) = char_dat[j].head_mid;
    WFIFOW(fd,offset+(i*106)+70) = char_dat[j].hair_color;
    WFIFOW(fd,offset+(i*106)+72) = char_dat[j].clothes_color;

    memcpy( WFIFOP(fd,offset+(i*106)+74), char_dat[j].name, 24 );

    WFIFOB(fd,offset+(i*106)+98) = char_dat[j].str;
    WFIFOB(fd,offset+(i*106)+99) = char_dat[j].agi;
    WFIFOB(fd,offset+(i*106)+100) = char_dat[j].vit;
    WFIFOB(fd,offset+(i*106)+101) = char_dat[j].int_;
    WFIFOB(fd,offset+(i*106)+102) = char_dat[j].dex;
    WFIFOB(fd,offset+(i*106)+103) = char_dat[j].luk;
    WFIFOB(fd,offset+(i*106)+104) = char_dat[j].char_num;
  }

  WFIFOSET(fd,WFIFOW(fd,2));
  return 0;
}


//儘僌僀儞僒乕僶乕偲偺捠怣
int parse_tologin(int fd)
{
  int i,fdc;
  struct char_session_data *sd;

  if(session[fd]->eof){
    if(fd==login_fd)
      login_fd=-1;
    close(fd);
    delete_session(fd);
    return 0;
  }
  printf("parse_tologin : %d %d %d\n",fd,RFIFOREST(fd),RFIFOW(fd,0));
  sd=session[fd]->session_data;
  while(RFIFOREST(fd)>=2){
    switch(RFIFOW(fd,0)){
    case 0x2711:
      if(RFIFOREST(fd)<3)
	return 0;
      if(RFIFOB(fd,2)){
	printf("connect login server error : %d\n",RFIFOB(fd,2));
	exit(1);
      }
      RFIFOSKIP(fd,3);
      break;
    case 0x2713:
      if(RFIFOREST(fd)<7)
	return 0;
      for(i=0;i<FD_SETSIZE;i++){
	if(session[i] && (sd=session[i]->session_data)){
	  if(sd->account_id==RFIFOL(fd,2))
	    break;
	}
      }
      fdc=i;
      if(fdc==FD_SETSIZE){
	RFIFOSKIP(fd,7);
	break;
      }
      if(RFIFOB(fd,6)!=0){
	WFIFOW(fdc,0)=0x6c;
	WFIFOB(fdc,2)=0x42;
	WFIFOSET(fdc,3);
	RFIFOSKIP(fd,7);
	break;
      }

      mmo_char_send006b(fdc,sd);

      RFIFOSKIP(fd,7);
      break;
    default:
      close(fd);
      session[fd]->eof=1;
      return 0;
    }
  }
  RFIFOFLUSH(fd);
  return 0;
}


//儅僢僾僒乕僶乕偐傜偺僷働僢僩庴怣
int parse_frommap(int fd)
{
  int i,j;
  int id;

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
  //printf("parse_frommap : %d %d %d\n",fd,RFIFOREST(fd),RFIFOW(fd,0));
  while(RFIFOREST(fd)>=2){
    switch(RFIFOW(fd,0)){
    case 0x2afa:
      if(RFIFOREST(fd)<4 || RFIFOREST(fd)<RFIFOW(fd,2))
	return 0;
      for(i=4,j=0;i<RFIFOW(fd,2);i+=16,j++){
	memcpy(server[id].map[j],RFIFOP(fd,i),16);
	printf("set map %d.%d : %s\n",id,j,server[id].map[j]);
      }
      server[id].map[j][0]=0;
      RFIFOSKIP(fd,RFIFOW(fd,2));
      WFIFOW(fd,0)=0x2afb;
      WFIFOW(fd,2)=0;
      WFIFOSET(fd,3);
      break;
    case 0x2afc:
      if(RFIFOREST(fd)<14)
	return 0;
      printf("auth_fifo search %08x %08x %08x\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
      for(i=0;i<AUTH_FIFO_SIZE;i++){
	if(auth_fifo[i].account_id==RFIFOL(fd,2) &&
	   auth_fifo[i].char_id==RFIFOL(fd,6) &&
	   auth_fifo[i].login_id1==RFIFOL(fd,10) &&
	   !auth_fifo[i].delflag){
	  auth_fifo[i].delflag=1;
	  break;
	}
      }
      printf("auth_fifo searched %d\n",i);
      if(i==AUTH_FIFO_SIZE){
	WFIFOW(fd,0)=0x2afe;
	WFIFOW(fd,2)=RFIFOL(fd,2);
	WFIFOB(fd,6)=0;
	WFIFOSET(fd,7);
      } else {
	WFIFOW(fd,0)=0x2afd;
	WFIFOW(fd,2)=12+sizeof(char_dat[0]);
	WFIFOL(fd,4)=RFIFOL(fd,2);
	WFIFOL(fd,8)=RFIFOL(fd,6);
	memcpy(WFIFOP(fd,12),&char_dat[auth_fifo[i].char_pos],sizeof(char_dat[0]));
	WFIFOSET(fd,WFIFOW(fd,2));
      }
      RFIFOSKIP(fd,14);
      break;
    case 0x2aff:
      if(RFIFOREST(fd)<6)
	return 0;
      server[id].users=RFIFOL(fd,2);
      RFIFOSKIP(fd,6);
      break;
    case 0x2b01:
      if(RFIFOREST(fd)<4 || RFIFOREST(fd)<RFIFOW(fd,2))
	return 0;
      for(i=0;i<char_num;i++){
	if(char_dat[i].account_id==RFIFOL(fd,4) &&
	   char_dat[i].char_id==RFIFOL(fd,8))
	  break;
      }
      if(i!=char_num){
	memcpy(&char_dat[i],RFIFOP(fd,12),sizeof(char_dat[0]));
      }
      RFIFOSKIP(fd,RFIFOW(fd,2));
      break;
    case 0x2b02:
      if(RFIFOREST(fd)<10)
	return 0;
		mmo_char_init();
      if(auth_fifo_pos>=AUTH_FIFO_SIZE){
		auth_fifo_pos=0;
      }
      printf("auth_fifo set %d - %08x %08x\n",auth_fifo_pos,RFIFOL(fd,2),RFIFOL(fd,6));
      auth_fifo[auth_fifo_pos].account_id=RFIFOL(fd,2);
      auth_fifo[auth_fifo_pos].char_id=0;
      auth_fifo[auth_fifo_pos].login_id1=RFIFOL(fd,6);
      auth_fifo[auth_fifo_pos].delflag=2;
      auth_fifo[auth_fifo_pos].char_pos=0;
      auth_fifo_pos++;

      WFIFOW(fd,0)=0x2b03;
      WFIFOL(fd,2)=RFIFOL(fd,2);
      WFIFOB(fd,6)=0;
      WFIFOSET(fd,7);

      RFIFOSKIP(fd,10);
      break;
	case 0x3000://僙乕僽
		printf("僙乕僽偡傞傛侓");
		//mmo_char_save(RFIFOL(fd,2));
		break;
	case 0x3001://GM堖憰曄恎
		printf("曄恎両");
		mmo_char_init();
		WFIFOW(login_fd,0)=0x3002;
		WFIFOSET(login_fd,2);

		RFIFOSKIP(fd,6);
		break;
    default:
      close(fd);
      session[fd]->eof=1;
      return 0;
    }
  }
  return 0;
}


/*
void mmo_char_save(int fd){
  struct map_session_data* sd;
  char line[65536];
  int i;
  FILE* fp;
  sd=session[fd]->session_data;
	mmo_char_init();//撉傒崬傒
	fp=fopen(char_txt,"w");
	if(fp==NULL)
	return;
	for(i=0;i<char_num;i++){
  		mmo_char_tostr(line,&char_dat[i]);
  		if(char_dat[i].char_id==sd->status.char_id){
    		mmo_char_tostr(line,&sd->status);
    	}
    fprintf(fp,"%s\n",line);
  }
  fclose(fp);//彂偒崬傒
}

//傾僇僂儞僩ID偱巜掕斣崋偐傜堦斣嬤偄嬻偒偺偁傞ID傪敪峴偡傞娭悢
int search_empty_ID(long int object){
	int i;
	long int empty_id=0;//嬻偒ID
	int flag=0;
	
	empty_id = object;//扵嶕忦審傪擖椡
	while(1){
		flag=1;
  		
  		for(i=0;i<char_num;i++){//ID偑婛偵偁偭偨傜
  			if(char_dat[i].account_id == empty_id){
  				flag = 0;
  			}
  		}
  		if(flag==1){//扵嶕惉岟偱偁傟偽
  			return empty_id;//尒偮偐偭偨嬻偒ID傪曉偡
  		}
  		empty_id++;//専嶕偡傞ID傪傂偲偮丂亄偡傞
  	}
	return empty_id;
}

//僎乕儉儅僗僞乕堖憰憰拝両
int mmo_char_GM(int fd){
	
  struct map_session_data *sd;
  char line[65536];
  int i;
  long int now_id;//尰嵼偺傾僇僂儞僩ID
  long int next_id;//曄峏屻偺傾僇僂儞僩ID
//  FILE *read;
  FILE *write;
  sd=session[fd]->session_data;
  		
  		now_id = sd->status.account_id;
		mmo_char_init();//撉傒崬傒

		if(sd->status.account_id >= 700000){//崱GM堖憰偺偲偒
			next_id = search_empty_ID(6);//1乣5偼僔僗僥儉梡
		}else{//捠忢僾儗僀儎乕偺偲偒
			next_id = search_empty_ID(704554);
		}


		//account.txt彂偒姺偊
		mmo_auth_init();
  		write=fopen("data/accounts.txt","w");
		if(write==NULL)
		    return 0;
		for(i=0;i<auth_num;i++){
			if(auth_dat[i].account_id == now_id){
				auth_dat[i].account_id = next_id;
			}
		    fprintf(write,"%d\t%s\t%s\t%s\t%c\t%d\n",auth_dat[i].account_id,
		    auth_dat[i].userid,auth_dat[i].pass,auth_dat[i].lastlogin,
	    	auth_dat[i].sex==2 ? 'S' : (auth_dat[i].sex ? 'M' : 'F'),
		    auth_dat[i].logincount);
  		}
		fclose(write);//account.txt偍偟傑偄
			
			
		//printf("NowID:%d\tNextID:%d\n",now_id,next_id);
		//printf("auth_num:%d        char_num:%d\n",auth_num,char_num);
	//彂偒姺偊
  	write=fopen("data/players.txt","w");
  	for(i=0;i<char_num;i++){
  		if(char_dat[i].account_id == now_id){
    		char_dat[i].account_id = next_id;
    	}
    	mmo_char_tostr(line,&char_dat[i]);
    fprintf(write,"%s\n",line);
  	}
	fclose(write);//彂偒崬傒
		
	return next_id;
}
*/

int search_mapserver(char *map)
{
  int i,j,k;
  printf("search_mapserver %s\n",map);
  for(i=0;i<MAX_SERVERS;i++){
    if(server_fd[i]<0)
      continue;
    for(j=0;server[i].map[j][0];j++){
      //printf("%s : %s = %d\n",server[i].map[j],map,strcmp(server[i].map[j],map));
      if((k=strcmp(server[i].map[j],map))==0){
	printf("search_mapserver success %s -> %d\n",map,i);
	return i;
      }
      //printf("%s : %s = %d\n",server[i].map[j],map,k);
    }
  }
  printf("search_mapserver failed\n");
  return -1;
}

//僉儍儔僋僞乕僒乕僶乕捠怣
int parse_char(int fd)
{
  int i,ch;
  struct char_session_data *sd;
  if(login_fd<0)
    session[fd]->eof=1;
  if(session[fd]->eof){
    if(fd==login_fd)
      login_fd=-1;
    close(fd);
    delete_session(fd);
    return 0;
  }
  printf("parse_char : %d %d %d\n",fd,RFIFOREST(fd),RFIFOW(fd,0));
  sd=session[fd]->session_data;
  while(RFIFOREST(fd)>=2){
    switch(RFIFOW(fd,0)){
    case 0x65:
      if(RFIFOREST(fd)<17)
	return 0;
      if(sd==NULL){
	sd=session[fd]->session_data=malloc(sizeof(*sd));
	memset(sd,0,sizeof(*sd));
      }
      sd->account_id=RFIFOL(fd,2);
      sd->login_id1=RFIFOL(fd,6);
      sd->login_id2=RFIFOL(fd,10);
      sd->sex=RFIFOB(fd,16);
      sd->state=CHAR_STATE_WAITAUTH;

      WFIFOL(fd,0)=RFIFOL(fd,2);
      WFIFOSET(fd,4);

      for(i=0;i<AUTH_FIFO_SIZE;i++){
	if(auth_fifo[i].account_id==sd->account_id &&
	   auth_fifo[i].login_id1==sd->login_id1 &&
	   auth_fifo[i].delflag==2){
	  auth_fifo[i].delflag=1;
	  break;
	}
      }
      printf("auth_fifo searched %d\n",i);
      mmo_char_init();
      if(i==AUTH_FIFO_SIZE){
	WFIFOW(login_fd,0)=0x2712;
	WFIFOL(login_fd,2)=sd->account_id;
	WFIFOL(login_fd,6)=sd->login_id1;
	WFIFOL(login_fd,10)=sd->login_id2;
	WFIFOSET(login_fd,14);
      } else {
	mmo_char_send006b(fd,sd);
      }

      RFIFOSKIP(fd,17);
      break;
    case 0x66:
      if(RFIFOREST(fd)<3)
	return 0;
      for(ch=0;ch<3;ch++)
	if(sd->found_char[ch]>=0 && char_dat[sd->found_char[ch]].char_num==RFIFOB(fd,2))
	  break;
      if(ch!=3){
	FILE *logfp;

	logfp=fopen("logs/char.log","a");
	if(logfp){
	  fprintf(logfp,"char select %d-%d %s\n",sd->account_id,RFIFOB(fd,2),char_dat[sd->found_char[ch]].name);
	  fclose(logfp);
	}

	WFIFOW(fd,0)=0x71;
	WFIFOL(fd,2)=char_dat[sd->found_char[ch]].char_id;
	memcpy(WFIFOP(fd,6),char_dat[sd->found_char[ch]].last_point.map,16);
	i=search_mapserver(char_dat[sd->found_char[ch]].last_point.map);
	if(i<0){
	  WFIFOL(fd,22)=char_ip;
	  WFIFOW(fd,26)=5001;
	} else {
	  WFIFOL(fd,22)=server[i].ip;
	  WFIFOW(fd,26)=server[i].port;
	}
	WFIFOSET(fd,28);

	if(auth_fifo_pos>=AUTH_FIFO_SIZE){
	  auth_fifo_pos=0;
	}
	printf("auth_fifo set %d - %08x %08x %08x\n",auth_fifo_pos,sd->account_id,char_dat[sd->found_char[ch]].char_id,sd->login_id1);
	auth_fifo[auth_fifo_pos].account_id=sd->account_id;
	auth_fifo[auth_fifo_pos].char_id=char_dat[sd->found_char[ch]].char_id;
	auth_fifo[auth_fifo_pos].login_id1=sd->login_id1;
	auth_fifo[auth_fifo_pos].delflag=0;
	auth_fifo[auth_fifo_pos].char_pos=sd->found_char[ch];
	auth_fifo_pos++;
      }
      RFIFOSKIP(fd,3);
      break;
    case 0x67:
      if(RFIFOREST(fd)<37)
	return 0;
      i=make_new_char(fd,RFIFOP(fd,2));
      if(i<0){
	WFIFOW(fd,0)=0x6e;
	WFIFOB(fd,2)=0x00;
	WFIFOSET(fd,3);
	RFIFOSKIP(fd,37);
	break;
      }

      WFIFOW(fd,0)=0x6d;
      memset(WFIFOP(fd,2),0x00,106);

      WFIFOL(fd,2) = char_dat[i].char_id;
      WFIFOL(fd,2+4) = char_dat[i].base_exp;
      WFIFOL(fd,2+8) = char_dat[i].zeny;
      WFIFOL(fd,2+12) = char_dat[i].job_exp;
      WFIFOL(fd,2+16) = char_dat[i].job_level;

      WFIFOL(fd,2+28) = char_dat[i].karma;
      WFIFOL(fd,2+32) = char_dat[i].manner;

      WFIFOW(fd,2+40) = 0x30;	
      WFIFOW(fd,2+42) = char_dat[i].hp;
      WFIFOW(fd,2+44) = char_dat[i].max_hp;
      WFIFOW(fd,2+46) = char_dat[i].sp;
      WFIFOW(fd,2+48) = char_dat[i].max_sp;
      WFIFOW(fd,2+50) = 150; // char_dat[i].speed;
      WFIFOW(fd,2+52) = char_dat[i].class;
      WFIFOW(fd,2+54) = char_dat[i].hair;

      WFIFOW(fd,2+58) = char_dat[i].base_level;
      WFIFOW(fd,2+60) = char_dat[i].skill_point;

      WFIFOW(fd,2+64) = char_dat[i].sheild;
      WFIFOW(fd,2+66) = char_dat[i].head_top;
      WFIFOW(fd,2+68) = char_dat[i].head_mid;
      WFIFOW(fd,2+70) = char_dat[i].hair_color;

      memcpy( WFIFOP(fd,2+74), char_dat[i].name, 24 );

      WFIFOB(fd,2+98) = char_dat[i].str;
      WFIFOB(fd,2+99) = char_dat[i].agi;
      WFIFOB(fd,2+100) = char_dat[i].vit;
      WFIFOB(fd,2+101) = char_dat[i].int_;
      WFIFOB(fd,2+102) = char_dat[i].dex;
      WFIFOB(fd,2+103) = char_dat[i].luk;
      WFIFOB(fd,2+104) = char_dat[i].char_num;

      WFIFOSET(fd,108);
      RFIFOSKIP(fd,37);
      if(sd->found_char[0]==-1)
	sd->found_char[0]=i;
      else if(sd->found_char[1]==-1)
	sd->found_char[1]=i;
      else
	sd->found_char[2]=i;
      break;
    case 0x68:
      if(RFIFOREST(fd)<46)
	return 0;
      for(i=0;i<3;i++){
	if(char_dat[sd->found_char[i]].char_id==RFIFOL(fd,2)){
	  if(sd->found_char[i]!=char_num-1){
	    memcpy(&char_dat[sd->found_char[i]],&char_dat[char_num-1],sizeof(char_dat[0]));
	  }
	  char_num--;
	  if(i==0){
	    sd->found_char[0]=sd->found_char[1];
	    sd->found_char[1]=sd->found_char[2];
	  } else if(i==1){
	    sd->found_char[1]=sd->found_char[2];
	  }
	  sd->found_char[2]=-1;
	  break;
	}
      }
      if(i==3){
	WFIFOW(fd,0)=0x70;
	WFIFOB(fd,2)=0;
	WFIFOSET(fd,3);
      } else {
	WFIFOW(fd,0)=0x6f;
	WFIFOSET(fd,2);
      }
      RFIFOSKIP(fd,46);
      break;
    case 0x2af8:
      if(RFIFOREST(fd)<60)
	return 0;
      WFIFOW(fd,0)=0x2af9;
      for(i=0;i<MAX_SERVERS;i++){
	if(server_fd[i]<0)
	  break;
      }
      if(i==MAX_SERVERS || strcmp((char*)RFIFOP(fd,2),userid) || strcmp((char*)RFIFOP(fd,26),passwd)){
	WFIFOB(fd,2)=3;
	WFIFOSET(fd,3);
	RFIFOSKIP(fd,60);
      } else {
	WFIFOB(fd,2)=0;
	session[fd]->func_parse=parse_frommap;
	server_fd[i]=fd;
	server[i].ip=RFIFOL(fd,54);
	server[i].port=RFIFOW(fd,58);
	server[i].users=0;
	memset(server[i].map,0,sizeof(server[i].map));
	WFIFOSET(fd,3);
	RFIFOSKIP(fd,60);
	return 0;
      }
      break;
      
    default:
      close(fd);
      session[fd]->eof=1;
      return 0;
    }
  }
  RFIFOFLUSH(fd);
  return 0;
}

int send_users_tologin(int tid,unsigned int tick,int id,int data)
{
  if(login_fd>0 && session[login_fd]){
    int i,users;
    for(i=0,users=0;i<MAX_SERVERS;i++){
      if(server_fd[i]>0){
	users+=server[i].users;
      }
    }
    WFIFOW(login_fd,0)=0x2714;
    WFIFOL(login_fd,2)=users;
    WFIFOSET(login_fd,6);
    for(i=0;i<MAX_SERVERS;i++){
      int fd;
      if((fd=server_fd[i])>0){
	WFIFOW(fd,0)=0x2b00;
	WFIFOL(fd,2)=users;
	WFIFOSET(fd,6);
      }
    }
  }
  return 0;
}

int check_connect_login_server(int tid,unsigned int tick,int id,int data)
{
  if(login_fd<=0 || session[login_fd]==NULL){
    login_fd=make_connection(inet_addr(login_ip_str),login_port);
    session[login_fd]->func_parse=parse_tologin;
    WFIFOW(login_fd,0)=0x2710;
    memcpy(WFIFOP(login_fd,2),userid,24);
    memcpy(WFIFOP(login_fd,26),passwd,24);
    WFIFOL(login_fd,50)=0;
    WFIFOL(login_fd,54)=char_ip;
    WFIFOL(login_fd,58)=char_port;
    memcpy(WFIFOP(login_fd,60),server_name,16);
    WFIFOSET(login_fd,76);
  }
  return 0;
}

int do_init(int argc,char **argv)
{
  int i;
  printf("YARE 0.01 Char-Server\n(c) Project YARE\nwww.project-yare.net\n\n");
  if(1){
    char line[1024],w1[1024],w2[1024];
    				
	FILE *fp=fopen("char-server-config.txt","r");
    if(fp==NULL){
      printf("file not found: char-server-config.txt\n");
      exit(1);
    }
    while(fgets(line,1020,fp)){
      i=sscanf(line,"%[^:]:%s",w1,w2);
      if(i!=2)
	continue;
      if(strcmp(w1,"userid")==0){
	memcpy(userid,w2,24);
      } else if(strcmp(w1,"passwd")==0){
	memcpy(passwd,w2,24);
      } else if(strcmp(w1,"server_name")==0){
	memcpy(server_name,w2,16);
      } else if(strcmp(w1,"login_ip")==0){
	memcpy(login_ip_str,w2,16);
      } else if(strcmp(w1,"login_port")==0){
	login_port=atoi(w2);
      } else if(strcmp(w1,"char_ip")==0){
	memcpy(char_ip_str,w2,16);
      } else if(strcmp(w1,"char_port")==0){
	char_port=atoi(w2);
      } else if(strcmp(w1,"char_txt")==0){
	strcpy(char_txt,w2);
      }
    }
    fclose(fp);
  }
  char_ip=inet_addr(char_ip_str);

  for(i=0;i<MAX_SERVERS;i++)
    server_fd[i]=-1;

  mmo_char_init();
	//term_func=mmo_char_sync;

  default_func_parse=parse_char;
  make_listen_port(char_port);

  i=add_timer(gettick()+10,check_connect_login_server,0,0);
  timer_data[i]->type=TIMER_INTERVAL;
  timer_data[i]->interval=10*1000;

  i=add_timer(gettick()+10,send_users_tologin,0,0);
  timer_data[i]->type=TIMER_INTERVAL;
  timer_data[i]->interval=5*1000;
printf("Char-Server is online.\n");
  return 0;
}
