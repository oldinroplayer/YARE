#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmo.h"
#include "grfio.h"
#include "itemdb.h"

#define ITEMDB_HASH_SIZE 64

static int itemdb_hash[ITEMDB_HASH_SIZE];
static int itemdb_size,itemdb_num;
static struct {
  // for hash
  int nameid;
  int next;

  // data
  int value;
  int type;
  int sell;
  int class;
  int equip;
  int weight;
  int atk;
  int def;
  int slot;
  int look;
} *itemdb;

static int search_itemdb_index(int nameid)
{
  int i;
  for(i=itemdb_hash[nameid%ITEMDB_HASH_SIZE];i>=0;i=itemdb[i].next)
    if(itemdb[i].nameid==nameid)
      return i;
  if(itemdb_size<=itemdb_num){
    itemdb_size+=ITEMDB_HASH_SIZE;
    itemdb=realloc(itemdb,sizeof(itemdb[0])*itemdb_size);
  }

  i=itemdb_num;
  itemdb_num++;
  itemdb[i].nameid=nameid;
  itemdb[i].next=itemdb_hash[nameid%ITEMDB_HASH_SIZE];
  itemdb_hash[nameid%ITEMDB_HASH_SIZE]=i;

  itemdb[i].value=10;
  itemdb[i].type=0;
  itemdb[i].class=0;
  itemdb[i].equip=0;
  itemdb[i].weight=10;
  itemdb[i].atk=0;
  itemdb[i].def=0;
  itemdb[i].slot=0;
  itemdb[i].look=0;
  return i;
}

/*×‚©‚­‰ü‘¢*/
int itemdb_type(int nameid)
{
  if(nameid>500 && nameid<600)
    return 0;	//heal item
  if(nameid>600 && nameid<700)
    return 2;	//use item
  if((nameid>700 && nameid<1100) ||
     (nameid>7000 && nameid<8000))
    return 3;	//correction
  if(nameid>=1750 && nameid<1760)
    return 10;	//arrow
  if(nameid>1100 && nameid<2000)
    return 4;	//weapon
  if((nameid>2100 && nameid<3000) ||
     (nameid>5000 && nameid<6000))
    return 5;	//armor
  if(nameid>4000 && nameid<5000)
    return 6;	//card
// ADDED on 04/08/2003 -----------------------
  if(nameid>=9001 && nameid<=9024)
    return 7;	//egg
	if(nameid>=10001 && nameid<=10019)
    return 8;	//pet equipment
// ---------------------------
  return 0;
}
/* ‘•”õ•i‚É‘Î‚µ‚Ä‚»‚ê‚ª‚Ç‚Ìƒ|ƒWƒVƒ‡ƒ“‚Å‚ ‚é‚©*/
int itemdb_stype(int nameid)
{
	//‚Q‚P‚O‚O`‚Q‚P‚O‚W‚Ü‚Å‚Í‚
	//‚Q‚R‚O‚P`‚Q‚U‚Q‚O‚Ü‚Å‚Í‚æ‚ë‚¢‹y‚ÑŒC‹y‚ÑƒAƒNƒZƒTƒŠ
	//‚T‚O‚O‚P`‚T‚O‚P‚X‚Ü‚Å‚Í‘S‚Ä“ª‘•”õ
	//2224,2225,2264‚à“ª‘•”õ
	//2201`229‚Ü‚Å‚Í“ª‘•”õi^‚ñ’†A‰ºAŠÜ‚Şj
	if(nameid>2100 && nameid<2109)
		return 8;//LOOK_SHEILD;
	//ã’i‘•”õ
	if((nameid>2205 && nameid<2212) || (nameid>2212 && nameid<2218)
		|| (nameid>2219 && nameid<2237) || (nameid>2243 && nameid<2262)
			|| (nameid>2272 && nameid<2276) || (nameid == 2277)
			|| (nameid>2278 && nameid<2280) ||(nameid>2281 && nameid<2286) 
			||(nameid>5000  && nameid < 5006) || (nameid > 5006 && nameid<5040) 
			||(nameid == 2264) ||(nameid>2286 && nameid<2288) 
			||(nameid>2288 && nameid<2291)
			|| (nameid>5041 && nameid<5043) || (nameid>5043 && nameid<5054)
			|| (nameid >2291 && nameid<2295) || (nameid >2295 && nameid <2300))
				return LOOK_HEAD_TOP;
	if((nameid >2200 && nameid<2206) || (nameid == 2212) 
		||( nameid == 2239) || (nameid == 2242) ||
		(nameid == 2243) || (nameid == 2262) ||
		(nameid == 2276) || (nameid == 2278) ||
		(nameid == 2263) || //’†A‰º‘•”õ‚à^‚ñ’†‚É‚µ‚Ä‚¨‚­
		(nameid == 2281) ||(nameid == 2288) || (nameid == 2291) || (nameid == 5006)
		|| (nameid == 2295) || (nameid == 5040) || (nameid == 5041) || (nameid == 5043) || (nameid == 2297))
				return LOOK_HEAD_MID;
	if((nameid == 2218) || (nameid ==  2219) ||( nameid == 2237 )||(nameid ==  2238) 
			|| (nameid == 2240) || (nameid == 2241)||( nameid>2264 && nameid<2271))
			return LOOK_HEAD_BOTTOM;
	if(nameid>1100 && nameid<2000)
   		 return 2;//LOOK_WEAPON;	//weapon
   		
   	return LOOK_06;
}

int itemdb_sell_init()
{
	FILE *fp;
	int item_id,type,price,sell;
	char name[128],JName[256];
	char line[2040];
	fp = fopen("config/item_db2.txt","r");
	if(fp)
	{
		while(fgets(line,2040,fp)){
			sscanf(line,"%d,%[^,],%[^,],%d,%d,%d",&item_id,name,JName,
				&type,&price,&sell);
			itemdb[search_itemdb_index(item_id)].sell = price;
		}
	}
	fclose(fp);
	return 0;
}

int itemdb_sellvalue(int nameid)
{
  return itemdb[search_itemdb_index(nameid)].sell;
}

int itemdb_weight(int nameid)
{
  return itemdb[search_itemdb_index(nameid)].weight;
}

int itemdb_isequip(int nameid)
{
  return itemdb[search_itemdb_index(nameid)].equip!=0;
}

//ƒAƒCƒeƒ€‚É‚½‚¢‚µ‚½‘•”õ‚Ìˆ×‚Ì”Ô†‚ğ•Ô‚·ŠÖ”
int itemdb_equip_point(int nameid,struct map_session_data *sd)
{
  return itemdb[search_itemdb_index(nameid)].equip;
}

static int itemdb_read_itemslottable(void)
{
  char *buf,*p;
  int s;
  buf=grfio_read("data\\itemslottable.txt");
  if(buf==NULL)
    return -1;
  s=grfio_size("data\\itemslottable.txt");
  buf[s]=0;
  for(p=buf;p-buf<s;){
    int nameid,equip;
    /* ƒAƒCƒeƒ€–¼‚Æ‘•”õ‚Ì‚½‚ß‚Ì”Ô†*/
    sscanf(p,"%d#%d#",&nameid,&equip);
    itemdb[search_itemdb_index(nameid)].equip=equip;
    p=strchr(p,10);
    if(!p) break;
    p++;
    p=strchr(p,10);
    if(!p) break;
    p++;
  }
  return 0;
}

int itemdb_init(void)
{
  int i;
  for(i=0;i<ITEMDB_HASH_SIZE;i++)
    itemdb_hash[i]=-1;
  itemdb_size=ITEMDB_HASH_SIZE;
  itemdb_num=0;
  itemdb=malloc(sizeof(itemdb[0])*itemdb_size);
  
  itemdb_read_itemslottable();
  itemdb_sell_init();
#if 0
  for(i=0;i<ITEMDB_HASH_SIZE;i++){
    int j,c;
    for(j=itemdb_hash[i],c=0;j>=0;j=itemdb[j].next,c++);
    printf("%4d",c);
    if(i%16==15) printf("\n");
  }
  exit(1);
#endif
  return 0;
}

int itemdb_view_point(int nameid)
{
	char strgat[1024 * 80],s_nameid[8],s_view[4];
	FILE *fp;
	int t_nameid,view,fcount,scount,i,tcount,j;
	memset(strgat,'\0',sizeof(strgat));
	memset(s_nameid,'\0',sizeof(s_nameid));
	memset(s_view,'\0',sizeof(s_view));
	scount = 0;
	fcount = 0;
	tcount = 0;
	//item_db.txt‚Ì‚h‚c‚©‚çŒ©‚½–Ú”Ô†i‚Q‚T”Ô–Új‚ğæ‚èo‚·
	fp = fopen("config/id_view.txt","r");
	while( (strgat[fcount++] =	getc(fp)) != EOF )
		;
	fclose(fp);
	while(strgat[scount] != '*')
	{
		i = 0;
		j = 0;
		while(strgat[scount] != ':' && i <=3){
			s_nameid[i] = strgat[scount];
			scount++;
			i++;
		}
		scount++;
		t_nameid = atoi(s_nameid);
		memset(s_view,'\0',sizeof(s_view));
		while(strgat[scount] != '\n'){
				s_view[j] = strgat[scount];
				scount++;
				j++;
			}

		view = atoi(s_view);
		scount++;
		if(t_nameid == nameid)
		{
			printf("ID:VI %d:%d\n",t_nameid,view);
			return view;
		}
	}
	printf("Miss Search Item ViewNumber\n");
	return 0;
}

struct item_db2 ret_item;

struct item_db2 item_database(int item_id)
{
	FILE *fp;
	//ID,Name,Jname,Type,Price,Sell,Weight,ATK,MATK,DEF,MDEF,Range,Slot,STR,AGI,VIT,INT,DEX
	//,LUK,Job,Gender,Loc,wLV,eLV,View,Ele,Eff,HP1,HP2,SP1,SP2,Rare,Box
	int name_id,type,price,sell,weight,atk,matk,def,mdef,range,slot,str,agi,vit,int_,dex;
	int luk,job,gender,loc,wlv,elv,view,ele,eff,hp1,hp2,sp1,sp2,rare,box;
	int hit,critical,flee,skill_id;
	int fcount,i;
	char name[128],JName[256];
	char line[2040];
	fcount = 0;
	i = 0;
	//item_db.txt‚Ì‚h‚c‚©‚çŒ©‚½–Ú”Ô†i‚Q‚T”Ô–Új‚ğæ‚èo‚·
	fp = fopen("config/item_db2.txt","r");
	if(fp)
	{
		//	printf("item_id %d\n",item_id);
		while(fgets(line,2040,fp)){
			sscanf(line,"%d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				&name_id,name,JName,
				&type,&price,&sell,&weight,&atk,&matk,&def,&mdef,&range,&slot,
				&str,&agi,&vit,&int_,&dex,&luk,&job,&gender,&loc,&wlv,
				&elv,&view,&ele,&eff,&hp1,&hp2,&sp1,&sp2,&rare,&box,&hit,&critical,&flee,&skill_id);
			if(name_id == item_id)
			{
			ret_item.nameid = name_id;
			ret_item.type = type;
			ret_item.sell = price;
			ret_item.weight = weight;
			ret_item.atk = atk;
			ret_item.matk = matk;
			ret_item.def = def;
			ret_item.mdef = mdef;
			ret_item.range = range;
			ret_item.slot = slot;
			ret_item.str = str;
			ret_item.agi = agi;
			ret_item.vit = vit;
			ret_item.int_ = int_;
			ret_item.dex = dex;
			ret_item.luk = luk;
			ret_item.job = job;
			ret_item.gender = gender;
			ret_item.loc = loc;
			ret_item.wlv = wlv;
			ret_item.elv = elv;
			ret_item.view = view;
			ret_item.ele = ele;
			ret_item.eff = eff;
			ret_item.hp1 = hp1;
			ret_item.hp2 = hp2;
			ret_item.sp1 = sp1;
			ret_item.sp2 = sp2;
			ret_item.hit = hit;
			ret_item.critical = critical;
			ret_item.flee = flee;
			ret_item.skill_id = skill_id;
			fclose(fp);
			return ret_item;
			}
		}
	}

	printf("THERE IS NOT SUCH ITEM!!!\n");
	fclose(fp);
	return ret_item;
}
