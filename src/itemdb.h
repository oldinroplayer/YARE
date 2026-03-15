int itemdb_type(int nameid);
int itemdb_sellvalue(int nameid);
int itemdb_weight(int nameid);
int itemdb_isequip(int named);
int itemdb_stype(int nameid);
int itemdb_equip_point(int named,struct map_session_data *sd);
int itemdb_init(void);
int itemdb_view_point(int nameid);
struct item_db2 item_database(int item_id);
