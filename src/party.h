int create_party(int fd,unsigned char *dat);
int mmo_party_init(void);
void mmo_party_sync(void);
int mmo_party_tostr(char *str,struct mmo_party *p);
int mmo_party_fromstr(char *str,struct mmo_party *p);
void mmo_party_do_init(void);

