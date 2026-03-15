// for map2.c
int npc_click(int fd,int npc_id);
int npc_menu_select(int fd,int npc_id,int sel);
int npc_next_click(int fd,int npc_id);
int npc_amount_input(int fd,int npc_id,int val);
int npc_close_click(int fd,int npc_id);
int npc_buysell_selected(int fd,int npc_id,int sell);
int npc_buy_selected(int fd,void *list,int num);
int npc_sell_selected(int fd,void *list,int num);
int read_npcdata(void);
int mons_walk(int tid,unsigned int tick,int m,int n);
void respawn_mons(int m,int n);
void respawn_mons2(int m,int n);
// ADDED on 04/09/2003
int* return_npc_current_id(void);

// for script.c
int mmo_map_npc_say(int fd, unsigned char* buf,unsigned long id, char *string);
int mmo_map_npc_next(int fd, unsigned char* buf, unsigned long id);
int mmo_map_npc_close(int fd, unsigned char* buf, unsigned long id);
int mmo_map_npc_select(int fd, unsigned char* buf, unsigned long id, char *string);
int mmo_map_npc_amount_request(int fd, unsigned char* buf, unsigned long id);
