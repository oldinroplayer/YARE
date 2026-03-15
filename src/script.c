#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mmo.h"
#include "core.h"
#include "npc.h"
#include "script.h"
#include "itemdb.h"
#include "map2.h"
#include "save.h"

#define SCRIPT_BLOCK_SIZE 256
//#define DEBUG_PARSER_OUTPUT
#define add_block_npc(m,n) {add_block(&map_data[m].npc[n]->block,m,map_data[m].npc[n]->x,map_data[m].npc[n]->y);map_data[m].npc[n]->block.type=BL_NPC;}

extern struct mmo_map_data map_data[];

enum {
  C_NOP,C_POS,C_INT,C_LOCAL,C_GLOBAL,C_PARAM,

  C_LOR,C_LAND,C_LE,C_LT,C_GE,C_GT,C_EQ,C_NE,	//operator
  C_XOR,C_OR,C_AND,C_ADD,C_SUB,C_MUL,C_DIV,C_MOD,C_NEG,C_LNOT,C_NOT,
  C_COUNTITEM,C_READPARAM,	// function

  C_MES,C_NEXT,C_CLOSE,C_MENU,C_MENU_GOTO,C_GOTO,C_JOBCHANGE,C_INPUT,C_IF,	//command
  C_SET,C_SETLOOK,C_WARP,C_GETITEM,C_DELITEM,C_CUTIN,C_VIEWPOINT,C_MONSTER,C_SAVE
};

struct {
  unsigned char *str;
  int com;
  unsigned char *arg;
} com_tbl[]={
  {"mes",C_MES,"s"},
  {"next",C_NEXT,""},
  {"close",C_CLOSE,""},
  {"menu",C_MENU,"m"},
  {"goto",C_GOTO,"l"},
  {"jobchange",C_JOBCHANGE,"i"},
  {"input",C_INPUT,""},
  {"warp",C_WARP,"sii"},
  {"setlook",C_SETLOOK,"ii"},
  {"set",C_SET,"Ie"},
  {"if",C_IF,"eg"},
  {"getitem",C_GETITEM,"ii"},
  {"delitem",C_DELITEM,"ii"},
  {"cutin",C_CUTIN,"si"},
  {"viewpoint",C_VIEWPOINT,"iiiii"},
  {"monster",C_MONSTER,"sii"},
  {"save",C_SAVE,"sii"},
  {NULL,0,NULL}
};
struct {
  unsigned char *str;
  int type;
  int val;
} alias_tbl[]={
  {"Job_Novice",C_INT,0},
  {"Job_Swordman",C_INT,1},
  {"Job_Mage",C_INT,2},
  {"Job_Archer",C_INT,3},
  {"Job_Acolyte",C_INT,4},
  {"Job_Merchant",C_INT,5},
  {"Job_Thief",C_INT,6},
  {"Job_Knight",C_INT,7},
  {"Job_Priest",C_INT,8},
  {"Job_Wizard",C_INT,9},
  {"Job_Blacksmith",C_INT,10},
  {"Job_Hunter",C_INT,11},
  {"Job_Assassin",C_INT,12},
  {"Job_Kami",C_INT,100},
  {"S_FLAG",C_LOCAL,0},
  {"Script_flag_num",C_PARAM,SP_SCRIPT_FLAG},
  {"SkillPoint",C_PARAM,SP_SKILLPOINT},
  {"StatusPoint",C_PARAM,SP_STATUSPOINT},
  {"Zeny",C_PARAM,SP_ZENY},
  {"BaseLevel",C_PARAM,SP_BASELEVEL},
  {"JobLevel",C_PARAM,SP_JOBLEVEL},
  {NULL,0}
};

int npc_id = 50000;
static unsigned char * script_buf;
static int script_pos,script_size;
int menu_goto[32];
char *str_buf;
int str_pos,str_size;
static struct {
  int str;
  int backpatch;
  int label;
  int next;
} *str_data;
int str_num=1,str_data_size;
int str_hash[16];
struct mons_data mons_data[4000];

int calc_hash(unsigned char *p)
{
  int h=0;
  while(*p){
    h=(h<<1)+(h>>3)+(h>>5)+(h>>8);
    h+=*p++;
  }
  return h&15;
}

// ｴ鈐ｸ､ﾎ､ﾇ､｢｡ｦEﾐﾈﾖｹ譯｢ﾌｵ､ｱ､｡ｦ｡ｦ1
int search_str(unsigned char *p)
{
  int i;
  i=str_hash[calc_hash(p)];
  while(i){
    if(strcmp(str_buf+str_data[i].str,p)==0){
      return i;
    }
    i=str_data[i].next;
  }
  return -1;
}
// ｴ鈐ｸ､ﾎ､ﾇ､｢｡ｦEﾐﾈﾖｹ譯｢ﾌｵ､ｱ､｡ｦﾐﾅﾐﾏｿ､ｷ､ﾆｿｷｵ｡ｦﾖｹ
int add_str(unsigned char *p)
{
  int i;
  i=calc_hash(p);
  if(str_hash[i]==0){
    str_hash[i]=str_num;
  } else {
    i=str_hash[i];
    for(;;){
      if(strcmp(str_buf+str_data[i].str,p)==0){
	return i;
      }
      if(str_data[i].next==0)
	break;
      i=str_data[i].next;
    }
    str_data[i].next=str_num;
  }
  if(str_num>=str_data_size){
    str_data_size+=128;
    str_data=realloc(str_data,sizeof(str_data[0])*str_data_size);
  }
  while(str_pos+strlen(p)+1>=str_size){
    str_size+=256;
    str_buf=realloc(str_buf,str_size);
  }
  strcpy(str_buf+str_pos,p);
  str_data[str_num].str=str_pos;
  str_data[str_num].next=0;
  str_data[str_num].backpatch=-1;
  str_data[str_num].label=-1;
  str_pos+=strlen(p)+1;
  return str_num++;
}

void check_script_buf(int size)
{
  if(script_pos+size>=script_size){
    script_size+=SCRIPT_BLOCK_SIZE;
    script_buf=realloc(script_buf,script_size);
    if(script_buf==NULL){
      printf("check_script_buf : error \n");
      exit(1);
    }
  }
}
void add_scriptb(int a)
{
  check_script_buf(1);
  script_buf[script_pos++]=a;
}

void add_scriptc(int a)
{
  while(a>=0x40){
    add_scriptb((a&0x3f)|0x40);
    a=(a-0x40)>>6;
  }
  add_scriptb(a&0x3f);
}

void add_scripti(int a)
{
  while(a>=0x40){
    add_scriptb(a|0xc0);
    a=(a-0x40)>>6;
  }
  add_scriptb(a|0x80);
}

void add_scriptl(int l)
{
  //printf("%d(%s) @ %d\n",l,str_buf+str_data[l].str,script_pos);
  check_script_buf(5);
  if(str_data[l].label<0){
    script_buf[script_pos++]=C_POS;
    script_buf[script_pos++]=str_data[l].backpatch;
    script_buf[script_pos++]=str_data[l].backpatch>>8;
    script_buf[script_pos++]=str_data[l].backpatch>>16;
    script_buf[script_pos++]=str_data[l].backpatch>>24;
    str_data[l].backpatch=script_pos-4;
  } else {
    script_buf[script_pos++]=C_POS;
    script_buf[script_pos++]=str_data[l].label;
    script_buf[script_pos++]=str_data[l].label>>8;
    script_buf[script_pos++]=str_data[l].label>>16;
    script_buf[script_pos++]=str_data[l].label>>24;
  }
}

void set_label(int l,int pos)
{
  int i,n;
  //printf("%d(%s) - %d\n",l,str_buf+str_data[l].str,pos);
  str_data[l].label=pos;
  for(i=str_data[l].backpatch;i>=0;){
    n=*(int*)(script_buf+i);
    script_buf[i]=pos;
    script_buf[i+1]=pos>>8;
    script_buf[i+2]=pos>>16;
    script_buf[i+3]=pos>>24;
    i=n;
  }
}

unsigned char *skip_space(unsigned char *p)
{
  while(1){
    while(isspace(*p))
      p++;
    if(p[0]=='/' && p[1]=='/'){
      while(*p && *p!='\n')
	p++;
    } else
      break;
  }
  return p;
}
unsigned char *skip_word(unsigned char *p)
{
  while(isalnum(*p)||*p=='_'|| *p>=0x81)
    if(*p>=0x81 && p[1]){
      p+=2;
    } else
      p++;
  return p;
}
int comcmp(unsigned char *str,unsigned char *com)
{
  while(*com && tolower(*str)==tolower(*com)){
    str++;
    com++;
  }
  return isalnum(*str)|| *str=='_' || *com;
}

unsigned char* parse_int(unsigned char *p)
{
  int i;

  //printf("parse_int : %s\n",p);
  p=skip_space(p);
  if(isdigit(*p) || *p=='-'){
    i=strtoul(p,(char**)&p,0);
    add_scripti(i);
  } else if((*p=='g' || *p=='l') && isdigit(p[1])){
    add_scriptc(*p=='g' ? C_GLOBAL : C_LOCAL);
    i=strtoul(p+1,(char**)&p,0);
    add_scripti(i);
  } else {
    for(i=0;alias_tbl[i].str;i++)
      if(comcmp(p,alias_tbl[i].str)==0)
	break;
    if(alias_tbl[i].str==NULL){
      printf("unknown constant %s\n",p);
      exit(1);
    }
    if(alias_tbl[i].type!=C_INT)
      add_scriptc(alias_tbl[i].type);
    add_scripti(alias_tbl[i].val);
    p=skip_word(p);
  }
  return p;
}

unsigned char* parse_subexpr(unsigned char*,int);

unsigned char* parse_simpleexpr(unsigned char *p)
{
  int func;

  if(*p=='('){
    p=parse_subexpr(p+1,-1);
    p=skip_space(p);
    if(*p++!=')'){
      printf("unmatch ')' : %s\n",p);
      exit(1);
    }
  } else if((func=C_COUNTITEM,comcmp(p,"countitem")==0)||
	    (func=C_READPARAM,comcmp(p,"readparam")==0)){
    p=skip_word(p);
    p=skip_space(p);
    if(*p!='('){
      printf("func reqest '(' ')' : %s\n",p);
      exit(1);
    }
    p=parse_subexpr(p+1,-1);
    p=skip_space(p);
    if(*p++!=')'){
      printf("func reqest '(' ')' : %s\n",p-1);
      exit(1);
    }
    add_scriptc(func);
  } else
    p=parse_int(p);
  return p;
}

unsigned char* parse_subexpr(unsigned char *p,int limit)
{
  int op,opl,len;

  p=skip_space(p);
  if((op=C_NEG,*p=='-') || (op=C_LNOT,*p=='!') || (op=C_NOT,*p=='~')){
    p=parse_subexpr(p+1,100);
    add_scriptc(op);
  }
  p=skip_space(p);
  p=parse_simpleexpr(p);
  p=skip_space(p);
  while(((op=C_ADD,opl=6,len=1,*p=='+') ||
	 (op=C_SUB,opl=6,len=1,*p=='-') ||
	 (op=C_MUL,opl=7,len=1,*p=='*') ||
	 (op=C_DIV,opl=7,len=1,*p=='/') ||
	 (op=C_MOD,opl=7,len=1,*p=='%') ||
	 (op=C_LAND,opl=1,len=2,*p=='&' && p[1]=='&') ||
	 (op=C_AND,opl=5,len=1,*p=='&') ||
	 (op=C_LOR,opl=0,len=2,*p=='|' && p[1]=='|') ||
	 (op=C_OR,opl=4,len=1,*p=='|') ||
	 (op=C_XOR,opl=3,len=1,*p=='^') ||
	 (op=C_EQ,opl=2,len=2,*p=='=' && p[1]=='=') ||
	 (op=C_NE,opl=2,len=2,*p=='!' && p[1]=='=') ||
	 (op=C_GE,opl=2,len=2,*p=='>' && p[1]=='=') ||
	 (op=C_GT,opl=2,len=1,*p=='>') ||
	 (op=C_LE,opl=2,len=2,*p=='<' && p[1]=='=') ||
	 (op=C_LT,opl=2,len=1,*p=='<')) && opl>limit){
    p+=len;
    p=parse_subexpr(p,opl);
    add_scriptc(op);
    p=skip_space(p);
  }
  return p;  /* return first untreated operator */
}

unsigned char* parse_expr(unsigned char *p)
{
  p=parse_subexpr(p,-1);
  add_scriptc(C_NOP);
  return p;
}

unsigned char* parse_arg(int com,unsigned char *p)
{
  int argc,c,i;
  unsigned char *tmp,bak;

  for(argc=0;com_tbl[com].arg[argc];argc++){
    p=skip_space(p);
    if(argc && *p==',')
      p++;
    p=skip_space(p);
    switch(com_tbl[com].arg[argc]){
    case 's':
      if(*p!='"'){
	printf("parse_arg : string must start \"\n");
	exit(1);
      }
      p++;
      while(*p && *p!='"'){
	if(p[-1]<=0x7e && *p=='\\')
	  p++;
	add_scriptb(*p++);
      }
      p++;
      add_scriptb(0);
      break;
    case 'm':
      for(c=0;;c++){
	if(*p!='"'){
	  printf("parse_arg : menu must start \"\n");
	  exit(1);
	}
	p++;
	while(*p && *p!='"'){
	  if(p[-1]<=0x7e && *p=='\\')
	    p++;
	  add_scriptb(*p++);
	}
	add_scriptb(':');
	p++;
	p=skip_space(p);
	if(*p!=','){
	  printf("parse_arg : not found ','\n");
	  exit(1);
	}
	p=skip_space(p+1);
	tmp=skip_word(p);
	bak=*tmp;
	*tmp=0;
	menu_goto[c]=add_str(p);
	*tmp=bak;
	p=skip_space(tmp);
	if(*p==','){
	  p=skip_space(p+1);
	} else {
	  c++;
	  break;
	}
      }
      add_scriptb(0);
      add_scriptc(C_MENU_GOTO);
      for(i=0;i<c;i++)
	add_scriptl(menu_goto[i]);
      add_scriptc(C_NOP);
      break;
    case 'l':
      tmp=skip_word(p);
      bak=*tmp;
      *tmp=0;
      add_scriptl(add_str(p));
      *tmp=bak;
      p=skip_space(tmp);
      break;
    case 'i':
      p=parse_int(p);
      break;
    case 'I':
      p=parse_int(p);
      break;
    case 'e':
      p=parse_expr(p);
      break;
    case 'g':
      p=skip_space(p);
      if(comcmp(p,"goto")){
	printf("if statement is 'if <cmp> goto <label>;' : %s\n",p);
	exit(1);
      }
      p=skip_space(p+4);

      // parse label
      tmp=skip_word(p);
      bak=*tmp;
      *tmp=0;
      add_scriptl(add_str(p));
      *tmp=bak;
      p=skip_space(tmp);

      break;
    default:
      printf("unknown arg type : '%c' @ %s\n",com_tbl[com].arg[argc],com_tbl[com].str);
      exit(1);
    }
  }
  return p;
}

unsigned char* parse_script(unsigned char *src)
{
  unsigned char *p,*tmp;
  int i;

  str_num=1;
  for(i=0;i<16;i++)
    str_hash[i]=0;

  script_buf=malloc(SCRIPT_BLOCK_SIZE);
  script_pos=0;
  script_size=SCRIPT_BLOCK_SIZE;

  p=src;
  p=skip_space(p);
  if(*p!='{'){
    printf("not found '{' : %c\n",*p);
    return NULL;
  }
  for(p++;*p && *p!='}';){
    p=skip_space(p);
    for(i=0;com_tbl[i].str;i++){
      if(comcmp(p,com_tbl[i].str)==0)
	break;
    }
    if(com_tbl[i].str){
      add_scriptc(com_tbl[i].com);
      p=skip_word(p);
      p=parse_arg(i,p);
    } else {
      tmp=skip_space(skip_word(p));
      if(*tmp==':'){
	int l;
	*skip_word(p)=0;
	l=add_str(p);
	set_label(l,script_pos);
	p=tmp+1;
	continue;
      } else {
	printf("parse_script : unknown command %s\n",p);
	return NULL;
      }
    }
    p=skip_space(p);
    if(*p!=';'){
      printf("not found ';' : %s\n",p);
      return NULL;
    }
    p=skip_space(p+1);
  }
  p=skip_space(p);
  if(*p!='}'){
    printf("not found '}' : %s\n",p);
    return NULL;
  }
  add_scriptc(C_CLOSE);	// for safety
#ifdef DEBUG_PARSER_OUTPUT
  for(i=0;i<script_pos;i++){
    if((i&15)==0) printf("%04x : ",i);
    printf("%02x ",script_buf[i]);
    if((i&15)==15) printf("\n");
  }
  printf("\n");
  //  exit(1);
#endif
  script_buf=realloc(script_buf,script_pos);
  return script_buf;
}

int unget_com_data=-1;

int get_com(unsigned char *script,int *pos)
{
  int i,j;
  if(unget_com_data>=0){
    i=unget_com_data;
    unget_com_data=-1;
    return i;
  }
  if(script[*pos]>=0x80){
    return C_INT;
  }
  i=0; j=0;
  while(script[*pos]>=0x40){
    i=script[(*pos)++]<<j;
    j+=6;
  }
  return i+(script[(*pos)++]<<j);
}

void unget_com(int c)
{
  if(unget_com_data!=-1){
    printf("unget_com can back one data\n");
  }
  unget_com_data=c;
}

int mmo_map_cutin(int fd,unsigned char* buf,char *img_name,int type)
{
  WBUFW(buf,0)=0x145;
  memcpy(WBUFP(buf,2),img_name,16);
  WBUFB(buf,18)=type;
  return 19;
}

int mmo_map_countitem(struct map_session_data *sd,int nameid)
{
  int i,count;

  for(i=count=0;i<MAX_INVENTORY;i++){
    if(sd->status.inventory[i].nameid==nameid)
      count+=sd->status.inventory[i].amount;
  }
  return count;
}

int mmo_map_readparam(struct map_session_data *sd,int i)
{
  switch(i){
  case SP_SKILLPOINT:
    return sd->status.skill_point;
  case SP_STATUSPOINT:
    return sd->status.status_point;
  case SP_ZENY:
    return sd->status.zeny;
  case SP_BASELEVEL:
    return sd->status.base_level;
  case SP_JOBLEVEL:
    return sd->status.job_level;
  case SP_SCRIPT_FLAG:
	return sd->local_reg[14];
  default:
    return 0;
  }
}

int get_num(struct map_session_data *sd,unsigned char *script,int *pos)
{
  int i,j,c;
  switch(c=get_com(script,pos)){
  case C_PARAM:
    i=get_num(sd,script,pos);
    return mmo_map_readparam(sd,i);
  case C_LOCAL:
  case C_GLOBAL:
    i=get_num(sd,script,pos);
    if(c==C_LOCAL){
      if(i>=0 && i<LOCAL_REG_NUM)
	return sd->local_reg[i];
    } else {
      if(i>=0 && i<GLOBAL_REG_NUM)
	return sd->status.global_reg[i];
    }
    return 0;
  case C_INT:
    i=0; j=0;
    while(script[*pos]>=0xc0){
      i+=(script[(*pos)++]&0x7f)<<j;
      j+=6;
    }
    return i+((script[(*pos)++]&0x7f)<<j);
  default:
    printf("get_num : unknown command %d\n",c);
    return 0;
  }
  return 0;
}

int get_expr(struct map_session_data *sd,unsigned char *script,int *pos)
{
  int stack[32],sp,c;

  for(sp=0;sp>=0 && (c=get_com(script,pos))!=C_NOP;){
    switch(c){
    case C_INT:
    case C_PARAM:
    case C_LOCAL:
    case C_GLOBAL:
      unget_com(c);
      stack[sp++]=get_num(sd,script,pos);
      break;

    case C_ADD:
      sp--;
      stack[sp-1]+=stack[sp];
      break;
    case C_SUB:
      sp--;
      stack[sp-1]-=stack[sp];
      break;
    case C_MUL:
      sp--;
      stack[sp-1]*=stack[sp];
      break;
    case C_DIV:
      sp--;
      if(stack[sp]==0)
	stack[sp]=1;
      stack[sp-1]/=stack[sp];
      break;
    case C_MOD:
      sp--;
      if(stack[sp]==0)
	stack[sp]=1;
      stack[sp-1]%=stack[sp];
      break;
    case C_NEG:
      stack[sp-1]=-stack[sp-1];
      break;

    case C_EQ:
      sp--;
      stack[sp-1]=stack[sp-1] == stack[sp];
      break;
    case C_NE:
      sp--;
      stack[sp-1]=stack[sp-1] != stack[sp];
      break;
    case C_GT:
      sp--;
      stack[sp-1]=stack[sp-1] > stack[sp];
      break;
    case C_GE:
      sp--;
      stack[sp-1]=stack[sp-1] >= stack[sp];
      break;
    case C_LT:
      sp--;
      stack[sp-1]=stack[sp-1] < stack[sp];
      break;
    case C_LE:
      sp--;
      stack[sp-1]=stack[sp-1] <= stack[sp];
      break;

    case C_LAND:
      sp--;
      stack[sp-1]=stack[sp-1] && stack[sp];
      break;
    case C_LOR:
      sp--;
      stack[sp-1]=stack[sp-1] || stack[sp];
      break;
    case C_LNOT:
      stack[sp-1]=!stack[sp-1];
      break;

    case C_AND:
      sp--;
      stack[sp-1]=stack[sp-1] & stack[sp];
      break;
    case C_OR:
      sp--;
      stack[sp-1]=stack[sp-1] | stack[sp];
      break;
    case C_XOR:
      sp--;
      stack[sp-1]=stack[sp-1] ^ stack[sp];
      break;
    case C_NOT:
      stack[sp-1]=~stack[sp-1];
      break;

    case C_COUNTITEM:
      stack[sp-1]=mmo_map_countitem(sd,stack[sp-1]);
      break;
    case C_READPARAM:
      stack[sp-1]=mmo_map_readparam(sd,stack[sp-1]);
      break;

    default:
      printf("get_expr : unknown command %d\n",c);
      return 0;
    }
  }
  if(sp!=1){
    if(sp<=0)
      printf("get_expr : stack empty???\n");
    else
      printf("get_expr : too many data???\n");
    return 0;
  }
  return stack[sp-1];
}

int run_script(int fd,struct map_session_data *sd)
{
  int stop,end,len;
  int j;
  int temp_map;
  int pos,c,i;
  int i1,i2;
  unsigned char *script,*s1;
  struct item tmp_item;

  pos=sd->npc_pc;
  script=map_data[sd->mapno].npc[sd->npc_n]->u.script;
  if(script==NULL){
    sd->npc_pc=0;
    sd->npc_id=0;
    sd->npc_n=0;
    return 0;
  }

  for(stop=0,end=0;!stop && !end;){
    switch(get_com(script,&pos)){
    case C_MES:
      len=mmo_map_npc_say(fd,WFIFOP(fd,0),sd->npc_id,script+pos);
      if(len>0) WFIFOSET(fd,len);
      pos+=strlen(script+pos)+1;
      break;
    case C_NEXT:
      len=mmo_map_npc_next(fd,WFIFOP(fd,0),sd->npc_id);
      if(len>0) WFIFOSET(fd,len);
      stop=1;
      break;
    case C_CLOSE:
      len=mmo_map_npc_close(fd,WFIFOP(fd,0),sd->npc_id);
      if(len>0) WFIFOSET(fd,len);
      end=1;
      break;
    case C_MENU:
      len=mmo_map_npc_select(fd,WFIFOP(fd,0),sd->npc_id,script+pos);
      if(len>0) WFIFOSET(fd,len);
      pos+=strlen(script+pos)+1;
      stop=1;
      break;
    case C_MENU_GOTO:
      for(i=1,c=C_POS;i<sd->local_reg[15] && (c=get_com(script,&pos))==C_POS;i++)
	pos+=4;
      if(c!=C_POS || (c=get_com(script,&pos))!=C_POS){
	end=1;
	break;
      }
      pos=*(int*)(script+pos);
      break;
    case C_JOBCHANGE:
      mmo_map_jobchange(fd,get_num(sd,script,&pos));
      break;
    case C_GOTO:
      if(get_com(script,&pos)!=C_POS){
	end=1;
	break;
      }
      pos=*(int*)(script+pos);
      break;
    case C_INPUT:
      len=mmo_map_npc_amount_request(fd,WFIFOP(fd,0),sd->npc_id);
      if(len>0) WFIFOSET(fd,len);
      stop=1;
      break;
    case C_SETLOOK:
		//アイテムナンバー
      i1=get_num(sd,script,&pos);
      i2=get_num(sd,script,&pos);
      len=mmo_map_set_look(fd,WFIFOP(fd,0),sd->account_id,itemdb_stype(i1),itemdb_view_point(i1));
      if(len>0) mmo_map_sendarea(fd,WFIFOP(fd,0),len,0);
      break;
    case C_WARP:
      s1=script+pos;
      pos+=strlen(script+pos)+1;
      i1=get_num(sd,script,&pos);
      i2=get_num(sd,script,&pos);
      mmo_map_changemap(fd,sd,s1,i1,i2,2);
      break;
    case C_SET:
      c=get_com(script,&pos);
      i1=get_num(sd,script,&pos);
      i2=get_expr(sd,script,&pos);
      switch(c){
      case C_PARAM:
	mmo_map_update_param(fd,i1,i2);
	break;
      case C_LOCAL:
	sd->local_reg[i1]=i2;
	break;
      case C_GLOBAL:
	sd->status.global_reg[i1]=i2;
	break;
      }
      break;
    case C_IF:
      i1=get_expr(sd,script,&pos);
      if(get_com(script,&pos)!=C_POS){
	end=1;
	break;
      }
      if(i1)
	pos=*(int*)(script+pos);
      else
	pos+=4;
      break;

    case C_GETITEM:
      i1=get_num(sd,script,&pos);
      i2=get_num(sd,script,&pos);
      memset(&tmp_item,0,sizeof(tmp_item));
      tmp_item.nameid=i1;
      tmp_item.amount=i2;
      tmp_item.identify=1;
      len=mmo_map_item_get(fd,WFIFOP(fd,0),&tmp_item);
      if(len>0) WFIFOSET(fd,len);
      break;
    case C_DELITEM:
      i1=get_num(sd,script,&pos);
      i2=get_num(sd,script,&pos);
      for(i=0;i<100;i++){
	if(sd->status.inventory[i].nameid==i1){
	  if(sd->status.inventory[i].amount>=i2){
	    len=mmo_map_item_lost(fd,WFIFOP(fd,0),i+2,i2);
	    if(len>0){
	      WFIFOSET(fd,len);
	      i2=0;
	    }
	  } else {
	    len=mmo_map_item_lost(fd,WFIFOP(fd,0),i+2,sd->status.inventory[i].amount);
	    if(len>0){
	      WFIFOSET(fd,len);
	      i2-=sd->status.inventory[i].amount;
	    }
	  }
	  break;
	}
      }
      if(i2>0)
	sd->local_reg[13]++;
      break;
    case C_CUTIN:
      s1=script+pos;
      pos+=strlen(script+pos)+1;
      i1=get_num(sd,script,&pos);
      len=mmo_map_cutin(fd,WFIFOP(fd,0),s1,i1);
      if(len>0) WFIFOSET(fd,len);
      break;
	case C_MONSTER:
		temp_map = sd->mapno;
		s1=script+pos;
		pos+=strlen(script+pos)+1;
		i1=get_num(sd,script,&pos);
		i2=get_num(sd,script,&pos);
		for(j=0;j <i2;j++)
		{
		map_data[temp_map].npc[map_data[temp_map].npc_num]=malloc(sizeof(struct npc_data));
	    map_data[temp_map].npc[map_data[temp_map].npc_num]->u.mons.speed=200;
		map_data[temp_map].npc[map_data[temp_map].npc_num]->class=i1;
		map_data[temp_map].npc[map_data[temp_map].npc_num]->id=npc_id++;
		map_data[temp_map].npc[map_data[temp_map].npc_num]->block.subtype=MONS;
		map_data[temp_map].npc[map_data[temp_map].npc_num]->u.mons.hp=10;
		map_data[temp_map].npc[map_data[temp_map].npc_num]->u.mons.script = 1;
		memcpy(map_data[temp_map].npc[map_data[temp_map].npc_num]->name,s1,24);
		respawn_mons2(temp_map,map_data[temp_map].npc_num);
		map_data[temp_map].npc_num++;
		}
		break;
    case C_VIEWPOINT:
      WFIFOW(fd,0)=0x144;
      WFIFOL(fd,2)=sd->npc_id;
      WFIFOL(fd,6)=get_num(sd,script,&pos);	// type
      WFIFOL(fd,10)=get_num(sd,script,&pos);	// x
      WFIFOL(fd,14)=get_num(sd,script,&pos);	// y
      WFIFOB(fd,18)=get_num(sd,script,&pos);	// point id
      WFIFOL(fd,19)=get_num(sd,script,&pos);	// color
      WFIFOSET(fd,23);
      break;
/*** Lemming ***/
	case C_SAVE:
	 test_storage(fd);
      s1=script+pos;
      pos+=strlen(script+pos)+1;
      i1=get_num(sd,script,&pos);
      i2=get_num(sd,script,&pos);
      strcpy(sd->status.save_point.map,s1);
	sd->status.save_point.x = i1;
	sd->status.save_point.y = i2;
	mmo_char_save(sd);
	break;
/*** Lemming ***/

    default:
      {
	FILE *log=fopen("map.log","a");
	if(log){
	  int i;
	  fprintf(log,"run_script error %04x : ",pos);
	  for(i=-5;i<=5;i++)
	    fprintf(log,"%02x%c",script[pos+i],(i==-1) ? '[':((i==0) ? ']':' '));
	  fprintf(log,"\n");
	  fclose(log);
	}
      }
      end=1;
      break;
    }
  }
  if(end){
    sd->npc_pc=0;
    sd->npc_id=0;
    sd->npc_n=0;
  } else {
    sd->npc_pc=pos;
  }
  return 0;
}