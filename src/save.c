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
#include "save.h"

int account_id_count = 1000000;
int server_num;

struct {
  int account_id,sex;
  char userid[24],pass[24],lastlogin[24];
  int logincount;
} *auth_dat;
int auth_num=0,auth_max=0;

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
char account_txt[256]="data/accounts.txt";

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


int mmo_auth_init(void)
{
  FILE *fp;
  int i,account_id,logincount;
  char line[1024],userid[24],pass[24],lastlogin[24],sex;
  fp=fopen(account_txt,"r");
  auth_dat=malloc(sizeof(auth_dat[0])*256);
  auth_num=0;
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
      if(i>=6){
			auth_dat[auth_num].logincount=logincount;
	  }else{
			auth_dat[auth_num].logincount=1;
	  }
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
  fp=fopen(account_txt,"w");
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
    return 1;	// ｲｼｰﾌｸﾟｴｹ
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

void mmo_char_save(struct map_session_data *sd){
  char line[65536];
  int i;
  FILE *fp;
	mmo_char_init();//読み込み
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
  fclose(fp);//書き込み
}

//アカウントIDで指定番号から一番近い空きのあるIDを発行する関数
int search_empty_ID(long int object){
	int i;
	long int empty_id=0;//空きID
	int flag=0;
	
	empty_id = object;//探索条件を入力
	while(1){
		flag=1;
  		
  		for(i=0;i<auth_num;i++){//IDが既にあったら
  			if(auth_dat[i].account_id == empty_id){
  				flag = 0;
  			}
  		}
  		if(flag==1){//探索成功であれば
  			return empty_id;//見つかった空きIDを返す
  		}
  		empty_id++;//検索するIDをひとつ　＋する
  	}
	return empty_id;
}


//ゲームマスター衣装装着！
int mmo_char_GM(struct map_session_data *sd){
  char line[65536];
  int i;
  long int now_id;//現在のアカウントID
  long int next_id;//変更後のアカウントID
//  FILE *read;
  FILE *write;
  		
  		now_id = sd->status.account_id;
		mmo_char_init();//読み込み
		mmo_auth_init();
		if(sd->status.account_id >= 700000){//今GM衣装のとき
			next_id = search_empty_ID(6);//1～5はシステム用
		}else{//通常プレイヤーのとき
			next_id = search_empty_ID(704554);
		}
		//account.txt書き換え
  		write=fopen(account_txt,"w");
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
		fclose(write);//account.txtおしまい
		printf("NowID:%ld\tNextID:%ld\n",now_id,next_id);
		printf("auth_num:%d        char_num:%d\n",auth_num,char_num);
	//chaos.txt書き換え
  	write=fopen(char_txt,"w");
  	for(i=0;i<char_num;i++){
  		if(char_dat[i].account_id == now_id){
    		char_dat[i].account_id = next_id;
    	}
    	mmo_char_tostr(line,&char_dat[i]);
    fprintf(write,"%s\n",line);
  	}
	fclose(write);//書き込み
		
	return next_id;
}

