#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "core.h"
#include "mmo.h"
#include "npc.h"
#include "itemdb.h"
#include "script.h"
#include "map2.h"
// ADDED on 04/09/2003 -------
#include "pet.h"
// ---------------------------

static int npc_id=50000;
extern char *npc_txt[];
extern int npc_txt_num;
extern char map[][16];
extern struct mmo_map_data map_data[];
extern int packet_len_table[];
extern struct mons_data mons_data[];

#define WARP_CLASS 45
#define add_block_npc(m,n) {add_block(&map_data[m].npc[n]->block,m,map_data[m].npc[n]->x,map_data[m].npc[n]->y); \
	map_data[m].npc[n]->block.type=BL_NPC;}

int mmo_map_npc_say(int fd, unsigned char* buf,unsigned long id, char *string)
{
  WBUFW(buf,0)=0xb4;
  WBUFW(buf,2)=strlen(string)+9;
  WBUFL(buf,4)=id;
  strncpy(WBUFP(buf,8), string,strlen(string)+1);
  return WBUFW(buf,2);
}

int mmo_map_npc_next(int fd, unsigned char* buf, unsigned long id)
{
  WBUFW(buf,0)=0xb5;
  WBUFL(buf,2)=id;
  return 6;
}

int mmo_map_npc_close(int fd, unsigned char* buf, unsigned long id)
{
  WBUFW(buf,0)=0xb6;
  WBUFL(buf,2)=id;
  return 6;
}

int mmo_map_npc_select(int fd, unsigned char* buf, unsigned long id, char *string)
{
  WBUFW(buf,0)=0xb7;
  WBUFW(buf,2)=strlen(string)+9;
  WBUFL(buf,4)=id;
  strncpy(WBUFP(buf,8), string, strlen(string)+1);
  return WBUFW(buf,2);
}

int mmo_map_npc_amount_request(int fd, unsigned char* buf, unsigned long id)
{
  WBUFW(buf,0)=0x142;
  WBUFL(buf,2)=id;
  return 6;
}

int mmo_map_npc_buysell(int fd, unsigned char* buf, unsigned long id)
{
  WBUFW(buf,0)=0xc4;
  WBUFL(buf,2)=id;
  return 6;
}

int mmo_map_npc_buy(int fd, unsigned char* buf, unsigned long id, struct npc_item_list *items, int dc_skill)
{
  int i;
  WBUFW(buf,0)=0xc6;
  for(i=0;items[i].nameid;i++){
    WBUFL(buf,4+i*11)=items[i].value;
    WBUFL(buf,8+i*11)=items[i].value;
    WBUFB(buf,12+i*11)=itemdb_type(items[i].nameid);
    WBUFW(buf,13+i*11)=items[i].nameid;
  }
  WBUFW(buf,2)=i*11+4;
  return i*11+4;
}

int mmo_map_npc_sell(int fd, unsigned char* buf, unsigned long id, int oc_skill)
{
  int i,c_item=0;
  struct map_session_data *sd;

  sd=session[fd]->session_data;
  WBUFW(buf,0)=0xc7;
  for(i=0;i<MAX_INVENTORY;i++) {
    if(sd->status.inventory[i].nameid) {
      WBUFW(buf,4+c_item*10)=i+2;
	  WBUFL(buf,6+c_item*10)=itemdb_sellvalue(sd->status.inventory[i].nameid);
      WBUFL(buf,10+c_item*10)=WBUFL(buf,6+c_item*10);
      c_item++;
    }
  }
  WBUFW(buf,2)=c_item*10+4;
  return c_item*10+4;
}

int npc_click(int fd,int npc_id)
{
  int npc_len,n;
  struct map_session_data *sd;
  sd=session[fd]->session_data;
  for(n=0;n<map_data[sd->mapno].npc_num;n++)
    if(map_data[sd->mapno].npc[n]->id==npc_id)
      break;
  if(n==map_data[sd->mapno].npc_num ||
     map_data[sd->mapno].npc[n]->block.type!=BL_NPC)
    return -1;


  switch(map_data[sd->mapno].npc[n]->block.subtype){
  case SCRIPT:
    sd->npc_pc=0;
    sd->npc_id=npc_id;
    sd->npc_n=n;
    run_script(fd,sd);
    break;
  case SHOP:
    npc_len = mmo_map_npc_buysell(fd, WFIFOP(fd,0),npc_id);
    WFIFOSET(fd, npc_len);
    sd->npc_id=npc_id;
    sd->npc_n=n;
    break;
  }
#if 0
  /*NPC test code*/
  if(npc_id==1) {
    npc_len = mmo_map_npc_say(fd, WFIFOP(fd,0),1,"test");

    WFIFOSET(fd, npc_len);

    mmo_map_npc_next(fd, WFIFOP(fd,0),1);
    WFIFOSET(fd, 6);
  }
#endif
  return 0;
}

int npc_menu_select(int fd,int npc_id,int sel)
{
  struct map_session_data *sd;
  sd=session[fd]->session_data;

  sd->local_reg[15]=sel;
  if(sel==0xff){
    sd->npc_pc=0;
    sd->npc_id=0;
    sd->npc_n=0;
    return 0;
  }
  run_script(fd,sd);

  return 0;
}

int npc_next_click(int fd,int npc_id)
{
  struct map_session_data *sd;
  sd=session[fd]->session_data;

  run_script(fd,sd);

  return 0;
}

int npc_amount_input(int fd,int npc_id,int val)
{
  struct map_session_data *sd;
  sd=session[fd]->session_data;

  sd->local_reg[14]=val;

  run_script(fd,sd);

  return 0;
}

int npc_close_click(int fd,int npc_id)
{
  return 0;
}

int npc_buysell_selected(int fd,int npc_id,int sell)
{
  int npc_len,n;
  struct map_session_data *sd;
  sd=session[fd]->session_data;

  for(n=0;n<map_data[sd->mapno].npc_num;n++)
    if(map_data[sd->mapno].npc[n]->id==npc_id)
      break;
  if(n==map_data[sd->mapno].npc_num ||
     map_data[sd->mapno].npc[n]->block.type!=BL_NPC ||
     map_data[sd->mapno].npc[n]->block.subtype!=SHOP ||
     sd->npc_id!=npc_id)
    return -1;

  if(sell){
    npc_len = mmo_map_npc_sell(fd, WFIFOP(fd,0),npc_id,0);
  } else {
    npc_len = mmo_map_npc_buy(fd, WFIFOP(fd,0),npc_id,map_data[sd->mapno].npc[n]->u.shop_item,0);
  }
  WFIFOSET(fd, npc_len);
  return 0;
}

struct map_session_data* make_rollback_point(struct map_session_data *sd)
{
  struct map_session_data *back;
  back=malloc(sizeof(*back));
  memcpy(back,sd,sizeof(*back));
  return back;
}

int do_rollback(struct map_session_data *sd,struct map_session_data *back)
{
  memcpy(sd,back,sizeof(*back));
  free(back);
  return 0;
}

int delete_rollback_point(struct map_session_data *back)
{
  free(back);
  return 0;
}

int npc_buy_selected(int fd,void *list,int num)
{
  int i,j,n,len,pos;
  struct item tmp_item;
  struct map_session_data *sd,*back;
  int fail=0;

  sd=session[fd]->session_data;
  for(n=0;n<map_data[sd->mapno].npc_num;n++)
    if(map_data[sd->mapno].npc[n]->id==sd->npc_id)
      break;
  if(n==map_data[sd->mapno].npc_num ||
     map_data[sd->mapno].npc[n]->block.type!=BL_NPC ||
     map_data[sd->mapno].npc[n]->block.subtype!=SHOP)
    return -1;

  back=make_rollback_point(sd);
  memset(&tmp_item,0,sizeof(tmp_item));
  for(i=pos=0;i<num;i++){
    tmp_item.amount=RBUFW(list,i*4); // amount
    tmp_item.nameid=RBUFW(list,i*4+2); // nameid
    tmp_item.identify=1;
    for(j=0;map_data[sd->mapno].npc[n]->u.shop_item[j].nameid;j++)
      if(map_data[sd->mapno].npc[n]->u.shop_item[j].nameid==tmp_item.nameid)
	break;
    if(map_data[sd->mapno].npc[n]->u.shop_item[j].nameid==0){
      fail=1;	// 今お話中のNPCで売ってないアイテムが指定されたっぽ
      session[fd]->eof=1;	// プチっ
      break;
    }
    len=mmo_map_item_get(fd,WFIFOP(fd,pos),&tmp_item);
    if(len<=0){
      if(len==-1)
	fail=3;	// 種類数超過。3万個越えも考えられるけど、面倒なので統一
      else if(len==-2)
	fail=2; // 重量オーバ
      else
	fail=3; // 謎
      break;
    }
    pos+=len;
    sd->status.zeny -= map_data[sd->mapno].npc[n]->u.shop_item[j].value*tmp_item.amount;
    if(sd->status.zeny < 0){
      fail=1;	// お金不足
      break;
    }
    pos+=mmo_map_set_param(fd,WFIFOP(fd,pos),SP_ZENY);
  }
  if(fail){
    do_rollback(sd,back);
  } else {
    WFIFOSET(fd,pos);
    delete_rollback_point(back);
  }
  WFIFOW(fd,0)=0xca;
  WFIFOB(fd,2)=fail;
  WFIFOSET(fd,3);
  return 0;
}

int npc_sell_selected(int fd,void *list,int num)
{
  int i,len,pos;
  struct map_session_data *sd,*back;
  int fail=0;

  sd=session[fd]->session_data;
  back=make_rollback_point(sd);
  for(i=pos=0;i<num;i++){
    sd->status.zeny+=itemdb_sellvalue(sd->status.inventory[RBUFW(list,i*4)-2].nameid)*RBUFW(list,i*4+2);
    pos+=mmo_map_set_param(fd,WFIFOP(fd,pos),SP_ZENY);
    len=mmo_map_item_lost(fd,WFIFOP(fd,pos),RBUFW(list,i*4),RBUFW(list,i*4+2));
    if(len<=0){
      fail=1;
      break;
    }
    pos+=len;
  }
  if(fail){
    do_rollback(sd,back);
  } else {
    WFIFOSET(fd,pos);
    delete_rollback_point(back);
  }
  WFIFOW(fd,0)=0xcb;
  WFIFOB(fd,2)=fail;
  WFIFOSET(fd,3);
  return 0;
}

int set_monster_random_point(int m,int n)
{
  int x,y;

  do {
    x=rand()%(map_data[m].xs-2)+1;
    y=rand()%(map_data[m].ys-2)+1;
  } while(map_data[m].gat[x+y*map_data[m].xs]==1 || map_data[m].gat[x+y*map_data[m].xs]==5);

  map_data[m].npc[n]->u.mons.to_x=map_data[m].npc[n]->x=x;
  map_data[m].npc[n]->u.mons.to_y=map_data[m].npc[n]->y=y;
  map_data[m].npc[n]->dir=0;

  return 0;
}

//見た目的にみえないところへ敵をとばす
int set_monster_no_point(int m,int n)
{
  int x,y;
    x=-1;
    y=-1;
  map_data[m].npc[n]->u.mons.to_x=map_data[m].npc[n]->x=x;
  map_data[m].npc[n]->u.mons.to_y=map_data[m].npc[n]->y=y;
  map_data[m].npc[n]->dir=0;
  return 0;
}

//モンスターが歩く関数
int mons_walk(int tid,unsigned int tick,int m,int n)
{
  int x,y,dx,dy;
  struct map_session_data sd;
  int ret,i;

  // fix point monster
  if(map_data[m].npc[n]->u.mons.speed<=0){
      map_data[m].npc[n]->u.mons.timer=
      add_timer(tick+rand()%5000+5000,mons_walk,m,n);
    return 0;
  }

  x=map_data[m].npc[n]->x;
  y=map_data[m].npc[n]->y;
  dx= map_data[m].npc[n]->u.mons.to_x - x;
  dy= map_data[m].npc[n]->u.mons.to_y - y;

 // printf("%d,%d,%d (%d,%d) (%d,%d)\n",m,n,map_data[m].npc[n]->id,map_data[m].npc[n]->x,map_data[m].npc[n]->y,dx,dy);
  if(dx || dy){
    // step
    if(dx<0) dx=-1;
    else if(dx>0) dx=1;
    if(dy<0) dy=-1;
    else if(dy>0) dy=1;

    if((x+dx)/BLOCK_SIZE != x/BLOCK_SIZE || (y+dy)/BLOCK_SIZE != y/BLOCK_SIZE){
      unsigned char buf[256];
      int bx,by;
      bx=x/BLOCK_SIZE;
      by=y/BLOCK_SIZE;
      WBUFW(buf,0)=0x80;
      WBUFL(buf,2)=map_data[m].npc[n]->id;
      WBUFB(buf,6)=0;	// no effect?
      if((x+dx)/BLOCK_SIZE != x/BLOCK_SIZE && bx-dx*AREA_SIZE>=0 && bx-dx*AREA_SIZE<map_data[m].bxs){ // x
	for(i=-AREA_SIZE;i<=AREA_SIZE;i++){ // x
	  if(by+i < 0 || by+i >= map_data[m].bys)
	    continue;
	  mmo_map_sendblock(m,bx-dx*AREA_SIZE,by+i,buf,packet_len_table[0x80],0,0);
	}
      }
      if((y+dy)/BLOCK_SIZE != y/BLOCK_SIZE && by-dy*AREA_SIZE>=0 && by-dy*AREA_SIZE<map_data[m].bys){ // y
	for(i=-AREA_SIZE;i<=AREA_SIZE;i++){ // y
	  if(bx+i < 0 || bx+i >= map_data[m].bxs)
	    continue;
	  mmo_map_sendblock(m,bx+i,by-dy*AREA_SIZE,buf,packet_len_table[0x80],0,0);
	}
      }
      del_block(&map_data[m].npc[n]->block);
      add_block(&map_data[m].npc[n]->block,m,x+dx,y+dy);

      memset(buf,0,256);
	  //表示範囲内モンスター情報
      WBUFW(buf,0)=0x7b;
      WBUFL(buf,2)=map_data[m].npc[n]->id;
      WBUFW(buf,6)=map_data[m].npc[n]->u.mons.speed;
      WBUFW(buf,14)=map_data[m].npc[n]->class;
      WBUFL(buf,22)=tick;
      set_2pos(WBUFP(buf,50),x,y,map_data[m].npc[n]->u.mons.to_x,map_data[m].npc[n]->u.mons.to_y);
      WBUFB(buf,55)=0;
      WBUFB(buf,56)=5;
      WBUFB(buf,57)=5;
      bx=(x+dx)/BLOCK_SIZE;
      by=(y+dy)/BLOCK_SIZE;
if((x+dx)/BLOCK_SIZE != x/BLOCK_SIZE && bx+dx*AREA_SIZE>=0 && bx+dx*AREA_SIZE<map_data[m].bxs){ // x
	for(i=-AREA_SIZE;i<=AREA_SIZE;i++){ // x
	  if(by+i < 0 || by+i >= map_data[m].bys)
	    continue;
	  mmo_map_sendblock(m,bx+dx*AREA_SIZE,by+i,buf,packet_len_table[0x7b],0,0);
	}
      }
      if((y+dy)/BLOCK_SIZE != y/BLOCK_SIZE && by+dy*AREA_SIZE>=0 && by+dy*AREA_SIZE<map_data[m].bys){ // y
	for(i=-AREA_SIZE;i<=AREA_SIZE;i++){ // y
	  if(bx+i < 0 || bx+i >= map_data[m].bxs)
	    continue;
	  mmo_map_sendblock(m,bx+i,by+dy*AREA_SIZE,buf,packet_len_table[0x7b],0,0);
	}
      }
    }
    map_data[m].npc[n]->x+=dx;
    map_data[m].npc[n]->y+=dy;

    dx= map_data[m].npc[n]->u.mons.to_x - map_data[m].npc[n]->x;
    dy= map_data[m].npc[n]->u.mons.to_y - map_data[m].npc[n]->y;
    if(dx==0 && dy==0){
      map_data[m].npc[n]->u.mons.timer=
	add_timer(tick+rand()%5000+5000,mons_walk,m,n);
    } else {
      map_data[m].npc[n]->u.mons.timer=
	add_timer(tick+map_data[m].npc[n]->u.mons.speed*((dx&&dy)?14:10)/10,mons_walk,m,n);
    }
    return 0;
  }

  // if no user in map, sleep
  if(map_data[m].users==0){
    map_data[m].npc[n]->u.mons.timer=
      add_timer(tick+rand()%5000+5000,mons_walk,m,n);
    return 0;
  }

  // search target
  //ターゲットサーチ
  //歩行可能な場所探索部分
	 for(i=0;i<5;i++){
		 dx=rand()%(12*2+1)-12;//次の座標Ｘ
		 dy=rand()%(12*2+1)-12;//次の座標Ｙ
	     ret=search_path(&sd,m,map_data[m].npc[n]->x,map_data[m].npc[n]->y,map_data[m].npc[n]->x+dx,map_data[m].npc[n]->y+dy,1);
		//ret==0ならば次へ進むことができますよ
		 if(ret==0)
			break;
		}
//	printf("N MoB X,Y,DX,DY ret %d %d %d %d %d\n",map_data[m].npc[n]->x,map_data[m].npc[n]->y,dx,dy,ret);
  if(ret==0){
    unsigned char buf[256];
    memset(buf,0,256);
	//移動先モンスターキャラクター設定
    WBUFW(buf,0)=0x7b;
    WBUFL(buf,2)=map_data[m].npc[n]->id;
    WBUFW(buf,6)=map_data[m].npc[n]->u.mons.speed;
    WBUFW(buf,14)=map_data[m].npc[n]->class;
    WBUFL(buf,22)=tick;
    set_2pos(WBUFP(buf,50),map_data[m].npc[n]->x,map_data[m].npc[n]->y,map_data[m].npc[n]->x+dx,map_data[m].npc[n]->y+dy);
    WBUFB(buf,55)=0;
    WBUFB(buf,56)=5;
    WBUFB(buf,57)=5;
    mmo_map_sendarea_mxy(m,map_data[m].npc[n]->x,map_data[m].npc[n]->y,buf,packet_len_table[0x7b]);
  	map_data[m].npc[n]->u.mons.to_x=map_data[m].npc[n]->x+dx;
    map_data[m].npc[n]->u.mons.to_y=map_data[m].npc[n]->y+dy;

    map_data[m].npc[n]->u.mons.timer=
	  add_timer(tick+map_data[m].npc[n]->u.mons.speed/2*((dx&&dy)?14:10)/10,mons_walk,m,n);
	return 0;
	 }
  map_data[m].npc[n]->u.mons.timer=
    add_timer(tick+rand()%5000+5000,mons_walk,m,n);
//  printf("SP0 speed %d\n",map_data[m].npc[n]->u.mons.timer);
  return 0;
}

//モンスター出現
void respawn_mons(int m,int n)
{
  unsigned char buf[256];

  del_block(&map_data[m].npc[n]->block);
  if(map_data[m].npc[n]->u.mons.script == 0){
	  if(map_data[m].npc[n]->u.mons.speed>0)
	    set_monster_random_point(m,n);
  }
  else
  {
	  if(map_data[m].npc[n]->u.mons.speed>0)
	    set_monster_no_point(m,n);
  }
  add_block_npc(m,n);
  mmo_map_set_npc(0,buf,map_data[m].npc[n]->id,map_data[m].npc[n]->class,map_data[m].npc[n]->x,map_data[m].npc[n]->y,0);
  mmo_map_sendarea_mxy(m,map_data[m].npc[n]->x,map_data[m].npc[n]->y,buf,packet_len_table[0x78]);
  map_data[m].npc[n]->u.mons.timer=
    add_timer(gettick()+rand()%5000+5000,mons_walk,m,n);
}
//モンスター出現2
void respawn_mons2(int m,int n)
{
  unsigned char buf[256];

  if(map_data[m].npc[n]->u.mons.speed>0)
    set_monster_random_point(m,n);
  add_block_npc(m,n);
  mmo_map_set_npc(0,buf,map_data[m].npc[n]->id,map_data[m].npc[n]->class,map_data[m].npc[n]->x,map_data[m].npc[n]->y,0);
  mmo_map_sendarea_mxy(m,map_data[m].npc[n]->x,map_data[m].npc[n]->y,buf,packet_len_table[0x78]);
  map_data[m].npc[n]->u.mons.timer=
    add_timer(gettick()+rand()%5000+5000,mons_walk,m,n);
}

int read_npcdata(void)
{
  int i,npc_txt_count;
  char line[2040];//1024];
  FILE *fp;

  // ADDED on 04/09/2003 -----------
  pet_store_init_npc_id(&npc_id);
  // -------------------------------
	printf("Loading config files...\n");
  // poring test data
  mons_data[1002].max_hp=45;
  mons_data[1002].base_exp=2;
  mons_data[1002].job_exp=1;
  mons_data[1002].dropitem[0].nameid=909;	//zeropy
  mons_data[1002].dropitem[0].p=8000;	// 80%

  // parse monster.txt
  fp=fopen("config/monster2.txt","r");
  if(fp){
	  while(fgets(line,2040,fp)){//1020,fp)){
      int class,lv,max_hp,base_exp,job_exp;
	  int  range,atk1,atk2,def1,def2,mdef1,mdef2,hit,flee;
	  int  scale,race,ele,mode,speed,adelay,amotion,dmotion;
      char name[128],JName[256],dropitem[1024],*p;

      //if(sscanf(line,"%d,%s,%s,%d,%d,%d,%d\t%[^\t]",
	  if(sscanf(line,"%d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\t]",
		  &class,name,JName,&lv,&max_hp,&base_exp,&job_exp,
		  &range,&atk1,&atk2,&def1,&def2,&mdef1,&mdef2,&hit,&flee,
		  &scale,&race,&ele,&mode,&speed,&adelay,&amotion,&dmotion,		  
		  dropitem)!=25)
	continue;
      if(class>=4000)
	continue;
      mons_data[class].max_hp=max_hp;
      mons_data[class].base_exp=base_exp;
      mons_data[class].job_exp=job_exp;
	  mons_data[class].lv = lv;
	  mons_data[class].range = range;
	  mons_data[class].atk1 = atk1;
	  mons_data[class].atk2 = atk2;
	  mons_data[class].def1 = def1;
	  mons_data[class].def2 = def2;
	  mons_data[class].mdef1 = mdef1;
	  mons_data[class].mdef2 = mdef2;
	  mons_data[class].hit	= hit;
	  mons_data[class].flee = flee;
	  mons_data[class].scale = scale;
	  mons_data[class].race = race;
	  mons_data[class].ele = ele;
	  mons_data[class].mode = mode;
	  mons_data[class].speed = speed;
	  mons_data[class].adelay = adelay;
	  mons_data[class].amotion = amotion;
	  mons_data[class].dmotion = dmotion;
      for(p=dropitem,i=0;p && i<16;i++,p=strchr(p,',')){
	int nameid,value;
	if(*p==',') p++;
	if(sscanf(p,"%d:%d",&nameid,&value)!=2)
	  break;
	mons_data[class].dropitem[i].nameid=nameid;
	mons_data[class].dropitem[i].p=value;
      }
    }
  }





  // parse npc.txt
  for(npc_txt_count=0;npc_txt_count<npc_txt_num;npc_txt_count++){
    fp=fopen(npc_txt[npc_txt_count],"r");
    if(fp==NULL)
      continue;
    while(fgets(line,1020,fp)){
      char mapname[256],w1[1024],w2[1024],w3[1024],w4[1024];
      int x,y,dir;
      if(sscanf(line,"%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]",w1,w2,w3,w4)!=4 || sscanf(w1,"%[^,],%d,%d,%d",mapname,&x,&y,&dir)!=4)
	continue;
      for(i=0;map[i][0];i++)
	if(strcmp(map[i],mapname)==0)
	  break;
      if(map[i][0]==0)
	continue;
      if(strcmp(w2,"warp")==0){
	int j,k,xs,ys,to_x,to_y;
	map_data[i].npc[map_data[i].npc_num]=malloc(sizeof(struct npc_data));
	map_data[i].npc[map_data[i].npc_num]->m=i;
	map_data[i].npc[map_data[i].npc_num]->x=x;
	map_data[i].npc[map_data[i].npc_num]->y=y;
	map_data[i].npc[map_data[i].npc_num]->dir=0;
	memcpy(map_data[i].npc[map_data[i].npc_num]->name,w3,24);

	map_data[i].npc[map_data[i].npc_num]->class=WARP_CLASS;
	map_data[i].npc[map_data[i].npc_num]->id=npc_id++;
	sscanf(w4,"%d,%d,%[^,],%d,%d",
	       &xs,&ys,
	       map_data[i].npc[map_data[i].npc_num]->u.warp.name,
	       &to_x,&to_y);
	xs++; ys++;
	map_data[i].npc[map_data[i].npc_num]->u.warp.x=to_x;
	map_data[i].npc[map_data[i].npc_num]->u.warp.y=to_y;
	map_data[i].npc[map_data[i].npc_num]->u.warp.xs=xs;
	map_data[i].npc[map_data[i].npc_num]->u.warp.ys=ys;
	for(j=0;j<ys;j++){
	  unsigned char *p;
	  p = &map_data[i].gat[x-xs/2 + (y-ys/2+j)* map_data[i].xs ];
	  for(k=0;k<xs;k++,p++){
	    if(*p==1 || *p==5)
	      continue;
	    *p|=0x80;
	  }
	}
	//printf("npc %s %d read done\n",map[i],map_data[i].npc[map_data[i].npc_num]->id);
	map_data[i].npc[map_data[i].npc_num]->block.subtype=WARP;
	add_block_npc(i,map_data[i].npc_num);
	map_data[i].npc_num++;
      } else if(strcmp(w2,"monster")==0){
	int j,class,num;
	sscanf(w4,"%d,%d",&class,&num);
	for(j=0;j<num;j++){
	  map_data[i].npc[map_data[i].npc_num]=malloc(sizeof(struct npc_data));
	  if(num==1 && x && y){
	    printf("(%d,%d) ",x,y);
	    map_data[i].npc[map_data[i].npc_num]->x=x;
	    map_data[i].npc[map_data[i].npc_num]->y=y;
	    map_data[i].npc[map_data[i].npc_num]->dir=0;
	    map_data[i].npc[map_data[i].npc_num]->u.mons.speed=-1;
	  } else {
	    set_monster_random_point(i,map_data[i].npc_num);
	    map_data[i].npc[map_data[i].npc_num]->u.mons.speed=200;
	  }
	  map_data[i].npc[map_data[i].npc_num]->class=class;
	  map_data[i].npc[map_data[i].npc_num]->id=npc_id++;
	  memcpy(map_data[i].npc[map_data[i].npc_num]->name,w3,24);
	  map_data[i].npc[map_data[i].npc_num]->u.mons.timer=
	    add_timer(gettick()+rand()%5000+5000,mons_walk,i,map_data[i].npc_num);
	  map_data[i].npc[map_data[i].npc_num]->block.subtype=MONS;
	map_data[i].npc[map_data[i].npc_num]->u.mons.hp=mons_data[map_data[i].npc[map_data[i].npc_num]->class].max_hp;
	map_data[i].npc[map_data[i].npc_num]->u.mons.script = 0;
	map_data[i].npc[map_data[i].npc_num]->u.mons.target_id = -1;
	map_data[i].npc[map_data[i].npc_num]->u.mons.current_attack_m = i;
	  add_block_npc(i,map_data[i].npc_num);
	  map_data[i].npc_num++;
	}
	//printf("monster %s read done\n",w3);
      } else if(strcmp(w2,"shop")==0){
	char *p;
	int max=32,pos=0;
	struct npc_item_list *item_list;

	map_data[i].npc[map_data[i].npc_num]=malloc(sizeof(struct npc_data));
	map_data[i].npc[map_data[i].npc_num]->m=i;
	map_data[i].npc[map_data[i].npc_num]->x=x;
	map_data[i].npc[map_data[i].npc_num]->y=y;
	map_data[i].npc[map_data[i].npc_num]->dir=dir;
	memcpy(map_data[i].npc[map_data[i].npc_num]->name,w3,24);
	map_data[i].npc[map_data[i].npc_num]->class=atoi(w4);
	map_data[i].npc[map_data[i].npc_num]->id=npc_id++;
	map_data[i].npc[map_data[i].npc_num]->block.subtype=SHOP;
	item_list=malloc(sizeof(item_list[0])*(max+1));
	p=strchr(w4,',');

	while(p && pos<max){
	  int nameid,value;
	  p++;
	  if(sscanf(p,"%d:%d",&nameid,&value)!=2)
	    break;
	  item_list[pos].nameid=nameid;
	  item_list[pos].value=value;
	  pos++;
	  p=strchr(p,',');
	}
	item_list=realloc(item_list,sizeof(item_list[0])*(pos+1));
	item_list[pos].nameid=0;
	map_data[i].npc[map_data[i].npc_num]->u.shop_item=item_list;

	add_block_npc(i,map_data[i].npc_num);
	//printf("npc %s %d read done\n",map[i],map_data[i].npc[map_data[i].npc_num]->id);
	map_data[i].npc_num++;
      } else if(strcmp(w2,"script")==0){
	char *buf;
	int size=65536,j;

	map_data[i].npc[map_data[i].npc_num]=malloc(sizeof(struct npc_data));
	map_data[i].npc[map_data[i].npc_num]->m=i;
	map_data[i].npc[map_data[i].npc_num]->x=x;
	map_data[i].npc[map_data[i].npc_num]->y=y;
	map_data[i].npc[map_data[i].npc_num]->dir=dir;
	memcpy(map_data[i].npc[map_data[i].npc_num]->name,w3,24);

	map_data[i].npc[map_data[i].npc_num]->class=atoi(w4);
	map_data[i].npc[map_data[i].npc_num]->id=npc_id++;
	map_data[i].npc[map_data[i].npc_num]->block.subtype=SCRIPT;

	buf=malloc(size);
	if(strchr(w4,'{'))
	  strcpy(buf,strchr(w4,'{'));
	else
	  buf[0]=0;
	while(1){
	  for(j=strlen(buf)-1;j>=0 && isspace(buf[j]);j--);
	  if(j>=0 && buf[j]=='}')
	    break;
	  fgets(line,1020,fp);
	  if(feof(fp))
	    break;
	  if(strlen(buf)+strlen(line)>=size){
	    size+=65536;
	    buf=realloc(buf,size);
	  }
	  if(strchr(buf,'{')==NULL){
	    if(strchr(line,'{'))
	      strcpy(buf,strchr(line,'{'));
	  } else
	    strcat(buf,line);
	}
	map_data[i].npc[map_data[i].npc_num]->u.script=parse_script(buf);

	add_block_npc(i,map_data[i].npc_num);
	//printf("npc %s %d read done\n",map[i],map_data[i].npc[map_data[i].npc_num]->id);
	map_data[i].npc_num++;
      }
    }
    fclose(fp);
  }
printf("Config files loaded.\n");
  return 0;
}

// ADDED on 04/09/2003 --------------------------------
int* return_npc_current_id(void)
{
	return &npc_id;
}

// ----------------------------------------------------
