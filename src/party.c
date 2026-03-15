#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmo.h"
#include "party.h"
#include "core.h"

int party_num;
int party_max;
struct mmo_party *party_dat;
char party_txt[24] = "config/party_data.txt";
int party_id_count;

/*=======================================================
 *	新規パーティー作成
 *-------------------------------------------------------
 */
int create_party(int fd,unsigned char *dat){
	FILE *logfp;
	int i;
	// struct map_session_data *sd;

  logfp=fopen("party.log","a");
  if(logfp){
    fprintf(logfp,"make new party %d %s\n",fd,dat);
    fclose(logfp);
  }
  // sd=session[fd]->session_data;
	printf("パーティー作る～HP\n");
  for(i=0; i<party_num; i++){
    if(strcmp(party_dat[i].party_name, (char *)dat)==0)//名前の重複がないかチェック
      break;
  }
  if(i!=party_num)
    return -1;
  	printf("メモリ確保\n");
  if(party_num>=party_max){
    party_max+=256;
    party_dat=realloc(party_dat,sizeof(party_dat[0])*party_max);
  }
  printf("値の代入\n");
  party_dat[i].party_id=party_id_count++;
  strcpy(party_dat[i].party_name, (char *)dat);
  party_num++;

  mmo_party_sync();
	return 1;
}

/*=======================================================
 *	パーティー情報配列　初期読み込み
 *-------------------------------------------------------
 */
int mmo_party_init(void){
  char line[1024];
  int ret;
  FILE *fp;
	party_num=0;
  fp=fopen(party_txt,"r");
  party_dat=malloc(sizeof(party_dat[0])*256);
  party_max=256;
  if(fp==NULL)
    return 0;
  while(fgets(line,1024,fp)){
    if(party_num>=party_max){
      party_max+=256;
      party_dat=realloc(party_dat,sizeof(party_dat[0])*party_max);
    }
    memset(&party_dat[party_num],0,sizeof(party_dat[0]));
    ret=mmo_party_fromstr(line,&party_dat[party_num]);
    if(ret){
      if(party_dat[party_num].party_id>=party_id_count)
	party_id_count=party_dat[party_num].party_id+1;
      party_num++;
    }
  }
  fclose(fp);
  return 0;
}

/*=======================================================
 *	パーティー情報書き込み
 *-------------------------------------------------------
 */
void mmo_party_sync(void){
  char line[1024];
  int i;
  FILE *fp;
  fp=fopen(party_txt,"w");
  if(fp==NULL)
    return;
  for(i=0;i<party_num;i++){
    mmo_party_tostr(line,&party_dat[i]);
    fprintf(fp,"%s\n",line);
  }
  fclose(fp);
}


int mmo_party_tostr(char *str,struct mmo_party *p){
	/*
	int i=0;
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
  */
  	  ;
  return 0;
}

int mmo_party_fromstr(char *str,struct mmo_party *p){

/*
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
 */
 	 ;
  return 1;
}

void mmo_party_do_init(void){
	party_id_count=0;
	party_num=0;
	mmo_party_init();
}

