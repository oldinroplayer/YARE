#define MAX_SERVERS 20
#define MAX_MAP_PER_SERVER 768
#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8
#define AREA_SIZE 3
#define MAX_INVENTORY 100
#define MAX_CART 100
#define MAX_SKILL 142
#define LOCAL_REG_NUM 16
#define GLOBAL_REG_NUM 16
#define DEFAULT_WALK_SPEED 140
#define LIFETIME_FLOORITEM 60

struct mmo_account {
  char* userid;
  char* passwd;
  int ver_1;
  int ver_2;

  long account_id;
  long login_id1;
  long login_id2;
  long char_id;
  char lastlogin[24];
  int sex;
};

struct mmo_char_server {
  char name[32];
  long ip;
  short port;
  int users;
};
struct mmo_map_server{
  long ip;
  short port;
  int users;
  char map[MAX_MAP_PER_SERVER][16];
};

struct item {
  int id;
  short nameid;
  short amount;
  short equip;
  char identify;
  char refine;
  char attribute;
  short card[4];
};
struct point{
  char map[16];
  short x,y;
};
/*******************************************
スキルＩＤ及びＬＶ箇所
*******************************************/
struct skill {
  unsigned char id,lv;
  int type_num;//連打回数
  int type_hit;//連打か単発か
  int type_inf;//パッシブであるかどうか等の情報
  int type_pl;//場所系の時にどのスキルにするか
  int type_nk;//攻撃系であるが敵を弱くしたりする魔法の場合設定
  int sp;
  int range;
};
struct skill_db {
  int id;
  int type_num;//連打回数
  int type_hit;//連打か単発か
  int type_inf;//パッシブであるかどうか等の情報
  int type_pl;//場所系の時にどのスキルにするか
  int type_nk;//攻撃系であるが敵を弱くしたりする魔法の場合設定
  int type_lv;//ＭＡＸＬＶ設定
  char type_require[128];
  int sp;
  int range;
};

//ID,Name,Jname,Type,Price,Sell,Weight,ATK,MATK,DEF,MDEF,Range,Slot,STR,AGI,VIT,INT,DEX,
//LUK,Job,Gender,Loc,wLV,eLV,View,Ele,Eff,HP1,HP2,SP1,SP2,Rare,Box
struct item_db2{
	int nameid;
	int type;
	int sell;
	int weight;
	int atk;
	int matk;
	int def;
	int mdef;
	int range;
	int slot;
	int str;
	int agi;
	int vit;
	int int_;
	int dex;
	int luk;
	int job;
	int gender;
	int loc;
	int wlv;
	int elv;
	int view;
	int ele;
	int eff;
	int hp1;
	int hp2;
	int sp1;
	int sp2;
	int hit,critical,flee,skill_id;
};

struct mmo_party_member {
	char nick[24];
	char map_name[16];
	int leader;
	int char_id;
	int x,y;
	int hp;
	int max_hp;
	int offline;
};

struct mmo_party {
	int party_id;
	char party_name[24];
	int exp;
	int item;
	int num;
	struct mmo_party_member member[12];
};

struct mmo_guild_member {
	int account_id;
	int char_id;
	int hear_style;
	int hear_color;
	int sex;
	int job;
	int lv;
	int present_exp;
	int online;
	int position;
	char nick[24];
};

struct mmo_guild {
	int guild_id;
	char guild_name[24];
	int guild_lv;
	int connum;
	int max_num;
	int avlage_lv;
	char guild_master[24];
	int exp;
	int next_exp;
	int emblem_id;
	struct mmo_guild_member member[16];
};

// ADDED on 04/09/2003 -------
struct pet_info
{
	// 画面上に表示されている場合は1,それ以外は0
	int activity;
	int pet_class;
	int pet_npc_id_on_map[MAX_MAP_PER_SERVER];
	int pet_id_as_npc;
	char pet_name[24];
	int pet_name_flag;
	int pet_level;
	int pet_hungry;
	int pet_friendly;
	int pet_accessory;
};
// ---------------------------

struct mmo_charstatus {
  int char_id;
  int account_id;
  int base_exp,job_exp,zeny;
  int heal_time;
  int sp_time;
  int damage_atk;

  short class;
  short status_point,skill_point;
  short hp,max_hp,sp,max_sp;
  short option,karma,manner;
  short party_id,guild_id;
  unsigned char	party_name[24],guild_name[24],class_name[24];//2003/2/22 追加
  short hair,hair_color,clothes_color;

  short weapon,sheild;
  short head_top,head_mid,head_bottom;

  char name[24];
  unsigned char base_level,job_level;
  unsigned char str,agi,vit,int_,dex,luk,char_num;
  unsigned char str2,agi2,vit2,int_2,dex2,luk2;
  //2002/2/22追加分
  unsigned short atk1,atk2,matk1,matk2,def1,def2,mdef1,mdef2,hit,flee1,flee2,critical;
  unsigned long aspd;
  //
  struct mmo_party_member party_member;
  struct mmo_guild_member guild_member;
  struct point last_point,save_point,memo_point[3];
  struct item inventory[MAX_INVENTORY],cart[MAX_CART];
  struct skill skill[MAX_SKILL];
  int global_reg[GLOBAL_REG_NUM];

  // ADDED on 04/09/2003 ---------
  struct pet_info pet;
  // -----------------------------
};


enum { BL_NUL, BL_PC, BL_NPC, BL_ITEM };
enum { MONS, WARP, SHOP, SCRIPT };
struct block_list {
  struct block_list *next,*prev;
  short type,subtype;
};

struct map_session_data {
  struct block_list block;
  struct {
    unsigned auth : 1 ;
    unsigned change_walk_target : 1 ;
  } state;
  int account_id,char_id,login_id1,login_id2,sex;
  struct mmo_charstatus status;
  int weight,max_weight;
  char mapname[16];
  int mapno;
  int fd;
  short x,y;
  short to_x,to_y;
  short speed;
  char sitting,dir,head_dir;
  unsigned long client_tick,server_tick;
  int torihiki_fd;//取引先相手のfd値、 2003/3/25追加
  int walkpath_len,walkpath_pos;
  int walktimer;
  char walkpath[64];
  int npc_id;
  int npc_n;
  int npc_pc;
  int local_reg[LOCAL_REG_NUM];
  unsigned long chatID;
  int search_timer;
  int attacktimer;
  int attacktarget;
};


struct npc_data {
  struct block_list block;
  int id;
  short m,x,y;
  short class,dir;
  char name[24];
  union {
    char *script;
    struct npc_item_list *shop_item;
    struct {
      short xs,ys;
      short x,y;
      char name[16];
    } warp;
    struct {
	  int attacktimer;//追加0222
	  int targettimer;
      int timer;//移動インターバル時間
      short to_x,to_y;
      int speed;
      int hp;
      int target_id;
	  int script;//スクリプト用敵出現処理(1だと復活しない)
	  int current_attack_m;
    } mons;
  } u;
};
struct npc_item_list {
  int nameid,value;
};

struct mmo_map_data {
  unsigned char *gat;
  struct block_list *block;
  int xs,ys;
  int bxs,bys;
  int npc_num;
  int users;
  struct npc_data *npc[MAX_NPC_PER_MAP];
};

struct flooritem_data {
  struct block_list block;
  int id;
  short m,x,y;
  short subx,suby;
  int drop_tick;
  struct item item_data;
};

enum {
  SP_SPEED,SP_BASEEXP,SP_JOBEXP,SP_KARMA,SP_MANNER,SP_HP,SP_MAXHP,SP_SP,
  SP_MAXSP,SP_STATUSPOINT,SP_0a,SP_BASELEVEL,SP_SKILLPOINT,SP_STR,SP_AGI,SP_VIT,
  SP_INT,SP_DEX,SP_LUK,SP_13,SP_ZENY,SP_15,SP_NEXTBASEEXP,SP_NEXTJOBEXP,
  SP_WEIGHT,SP_MAXWEIGHT,SP_1a,SP_1b,SP_1c,SP_1d,SP_1e,SP_1f,
  SP_USTR,SP_UAGI,SP_UVIT,SP_UINT,SP_UDEX,SP_ULUK,SP_26,SP_27,
  SP_28,SP_ATK1,SP_ATK2,SP_MATK1,SP_MATK2,SP_DEF1,SP_DEF2,SP_MDEF1,
  SP_MDEF2,SP_HIT,SP_FLEE1,SP_FLEE2,SP_CRITICAL,SP_ASPD,SP_36,SP_JOBLEVEL,SP_SCRIPT_FLAG
};

enum {
  LOOK_BASE,LOOK_01,LOOK_WEAPON,LOOK_HEAD_BOTTOM,LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_06,LOOK_07,LOOK_SHEILD
};

struct mmo_chat
{
  unsigned short  len;      /* */
  unsigned long ownID;      /* owner ID */
  unsigned long chatID;     /* chat ID */
  unsigned short  limit;      /* join limit */
  unsigned short  users;      /* ? */
  unsigned char pub;      /* room attribute */
  unsigned char title[62];    /* room title MAX 60 */
  unsigned char pass[8];    /* password */
  int usersfd[20];
  unsigned long usersID[20];  /* join users ID */
  unsigned char usersName[20][24]; /* join users Name */
  struct mmo_chat* next;
  struct mmo_chat* prev;
};

struct mons_data {
  int type;
  int max_hp;
  int npc_num;
  int job_exp;
  int base_exp;
  int lv;
  int  range,atk1,atk2,def1,def2,mdef1,mdef2,hit,flee;
  int  scale,race,ele,mode,speed,adelay,amotion,dmotion;
  struct {
    int nameid,p;
  } dropitem[16];
};
