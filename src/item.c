{
/****************      適当なアイテムIDを探す         ***************************************************************/
int search_item(int object){
	char file_name[40];
	char strgat[80];
	char gomi[40];
	FILE *fp;
	int scount=0,fcount=0,nameid=0;
	
	printf("object:%d\n",object);
	if(object==1){
		strcpy(file_name,"item_all.list");
		scount = (int)(((double)rand()/(double)RAND_MAX)*1061.0);
		printf("scount:%d\n",scount);
	}else if(object==2){
		strcpy(file_name,"item_equipment.list");
		scount = (int)(((double)rand()/(double)RAND_MAX)*490.0);
	}else if(object==3){
		strcpy(file_name,"item_card.list");
		scount = (int)(((double)rand()/(double)RAND_MAX)*149.0);
	}else if(object==4){
		strcpy(file_name,"item_present.list");
		scount = (int)(((double)rand()/(double)RAND_MAX)*77.0);
	}
	if((fp = fopen(file_name,"r"))==NULL){
		printf("オープンできねえぞ、(ﾟДﾟ )ｺﾞﾙｧ!!\n");
	}
	for(fcount=0;fcount<scount;fcount++){//ファイルのscount行目まですすむ
		fgets(strgat,80,fp);
	}
	fclose(fp);
	sscanf(strgat,"%d%s",&nameid,gomi);
	printf("アイテムID:%d          アイテム名:%s\n",nameid,gomi);
	return nameid;
}


/**************   指定したアイテムIDが存在するかチェック   ************************************/
int is_item(int object){
	char str[80];
	FILE *fp;
	int item_id=0,result=0;
	
	if((fp = fopen("item_all.list","r"))==NULL){
		printf("オープンできねえぞ、(ﾟДﾟ )ｺﾞﾙｧ!!\n");
	}
	while(fgets(str,80,fp)!=NULL){
		sscanf(str,"%d",&item_id);
		if(item_id==object){
			result=1;//見つかった
				break;
		}
	}
	fclose(fp);
	return result;
}
/**********************************************************************************************/

/**************   アイテム使用    *************************************************************/
     	int nameid=p->inventory[RFIFOW(fd,2)-2].nameid;
     	short option;
     	double bonus=0;//Vitによる回復量補正
     	item_num = itemdb_type(p->inventory[RFIFOW(fd,2)-2].nameid);//inventory　持ち物
  		p=&sd->status;
     	if(item_num==0){//回復アイテムならば
			WFIFOW(fd,0) = 0xa8;
			WFIFOW(fd,2) = RFIFOW(fd,2);
			WFIFOW(fd,4) = --p->inventory[RFIFOW(fd,2)-2].amount;
			WFIFOB(fd,6) = 01;
			WFIFOSET(fd,7);
			if(p->inventory[RFIFOW(fd,2)-2].amount <= 0){
				p->inventory[RFIFOW(fd,2)-2].nameid=0;
			}
			//回復量　=　（アイテムの基本回復量の1つ）*（1 + VIT/100 + 回復スキルレベル/10)
				if(p->skill[3].lv >= 1){
					bonus = ((double)p->skill[3].lv)/10.0;
				}
				bonus += ((double)(p->vit+100)/(double)100);
			//srand((unsigned)time(NULL));//乱数生成
			if(nameid==501){//赤ポーション
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 44)+30) * bonus;
			}else if(nameid==502){//紅ポーション
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 69)+50) * bonus;
			}else if(nameid==503){//黄色ポーション
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 104)+80) * bonus;
			}else if(nameid==504){//白ポーション
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 120)+149) * bonus;
			}else if(nameid==505){//青ポーション
				p->sp += (double)((((double)rand()/(double)RAND_MAX) * 45)+53);
			}else if(nameid==506){//緑ポーション　　毒回復
				option = 0x0038;
				p->option = p->option & option;//ペコ、カート、鷹以外のフラグ消す
				WFIFOW(fd,0)=0x0119;
				WFIFOL(fd,2)=sd->account_id;
				WFIFOW(fd,6)=0;
				WFIFOW(fd,8)=0;
				WFIFOW(fd,10)=p->option;
			mmo_map_sendarea( fd, WFIFOP(fd,0),packet_len_table[0x0119], 0 );
			}else if(nameid==507){//赤ハーブ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 12)+19) * bonus;
			}else if(nameid==508){//黄色ハーブ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 21)+29) * bonus;
			}else if(nameid==509){//白ハーブ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 50)+50) * bonus;
			}else if(nameid==510){//青ハーブ
				p->sp += (double)((((double)rand()/(double)RAND_MAX) * 10)+17);
			}else if(nameid==511){//緑ハーブ　　毒回復
				option = 0x0038;
				p->option = p->option & option;//ペコ、カート、鷹以外のフラグ消す
				WFIFOW(fd,0)=0x0119;
				WFIFOL(fd,2)=sd->account_id;
				WFIFOW(fd,6)=0;
				WFIFOW(fd,8)=0;
				WFIFOW(fd,10)=p->option;
			mmo_map_sendarea( fd, WFIFOP(fd,0),packet_len_table[0x0119], 0 );
			}else if(nameid==512){//りんご
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 12)+15) * bonus;
			}else if(nameid==513){//バナナ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 11)+15) * bonus;
			}else if(nameid==514){//ブドウ
				p->sp += (double)((((double)rand()/(double)RAND_MAX) * 8)+11);
			}else if(nameid==515){//にんじん
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 13)+15) * bonus;
			}else if(nameid==516){//芋
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 10)+14) * bonus;
			}else if(nameid==517){//肉
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 41)+55) * bonus;
			}else if(nameid==518){//蜂の蜜
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 46)+66) * bonus;
				p->sp += (double)((((double)rand()/(double)RAND_MAX) * 5)+9);
			}else if(nameid==519){//ミルク
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 15)+22) * bonus;
			}else if(nameid==520){//ヒナレの葉
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 60)+22) * bonus;
			}else if(nameid==521){//アロエの葉
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 150)+25) * bonus;
			}else if(nameid==522){//マステラの実
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 200)+200) * bonus;
			}else if(nameid==523){//聖水　　ゾンビ解除
				option = 0x0038;
				p->option = p->option & option;//ペコ、カート、鷹以外のフラグ消す
				WFIFOW(fd,0)=0x0119;
				WFIFOL(fd,2)=sd->account_id;
				WFIFOW(fd,6)=0;
				WFIFOW(fd,8)=0;
				WFIFOW(fd,10)=p->option;
			mmo_map_sendarea( fd, WFIFOP(fd,0),packet_len_table[0x0119], 0 );
			}else if(nameid==525){//万能薬　　異常状態回復
				option = 0x0038;
				p->option = p->option & option;//ペコ、カート、鷹以外のフラグ消す
				WFIFOW(fd,0)=0x0119;
				WFIFOL(fd,2)=sd->account_id;
				WFIFOW(fd,6)=0;
				WFIFOW(fd,8)=0;
				WFIFOW(fd,10)=p->option;
			mmo_map_sendarea( fd, WFIFOP(fd,0),packet_len_table[0x0119], 0 );
			}else if(nameid==526){//ローヤルゼリー　　HP、SP全回復
				p->sp = p->max_sp;
				p->hp = p->max_hp;
			}else if(nameid==528){//化け物の餌
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 41)+55) * bonus;
			}else if(nameid==529){//キャンディ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 30)+44) * bonus;
			}else if(nameid==530){//スティックキャンディ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 60)+88) * bonus;
			}else if(nameid==531){//リンゴジュース
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 15)+22) * bonus;
			}else if(nameid==532){//バナナジュース
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 15)+22) * bonus;
			}else if(nameid==533){//ブドウジュース
				p->sp += (double)((((double)rand()/(double)RAND_MAX) * 30)+22);
			}else if(nameid==534){//にんじんジュース
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 15)+22) * bonus;
			}else if(nameid==535){//カボチャ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 12)+15) * bonus;
			}else if(nameid==536){//アイスクリーム
				p->sp += (double)((((double)rand()/(double)RAND_MAX) * 150)+10) * bonus;
			}else if(nameid==538){//よく焼いたクッキー
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 500)+10) * bonus;
			}else if(nameid==539){//ひとくちケーキ
				p->hp += (double)((((double)rand()/(double)RAND_MAX) * 1500)+10) * bonus;
			}

			if(p->hp > p->max_hp){//最大HPを超えたら
				p->hp = p->max_hp;//最大HP丁度に設定しなおす
			}
			if(p->sp > p->max_sp){//最大SPを超えたら
				p->sp = p->max_sp;//最大SP丁度に設定しなおす
			}
			
/*
R 00b0 <type>.w <val>.l
	色々な能力値の更新。以下type:対応する数値を列挙
	0000:speed 0003:悪行値 0004:マナーポイント 0005:HP 0006:MaxHP
	0007:SP 0008:MaxSP 0009:ステータスポイント 000b:ベースレベル
	000c:スキルポイント 0018:重量(表示されてる数字の10倍)
	0019:最大重量(表示されてる数字の10倍)
	0029:ATK前 002a:ATK後 002b:MATK前 002c:MATK後
	002d:DEF前 002e:DEF後 002f:MDEF前 0030:MDEF後
	0031:HIT 0032:FLEE前 0033:FLEE後 0034:クリティカル
	0035:ASPD(2ms単位の時間?) 0037:ジョブレベル
	0082:謎 ATK後と同じ数字?
*/
				WFIFOW(fd,0) = 0xb0;
				WFIFOW(fd,2) = 0005;
				WFIFOL(fd,4) = p->hp;
				WFIFOSET(fd,8);
				
				WFIFOW(fd,0) = 0xb0;
				WFIFOW(fd,2) = 0007;
				WFIFOL(fd,4) = p->sp;
				WFIFOSET(fd,8);
				
     }else if(item_num==2){//特殊効果を持つ使用アイテムならば
     	 	WFIFOW(fd,0) = 0xa8;
			WFIFOW(fd,2) = RFIFOW(fd,2);
			WFIFOW(fd,4) = --p->inventory[RFIFOW(fd,2)-2].amount;
			WFIFOB(fd,6) = 01;
			WFIFOSET(fd,7);
			if(p->inventory[RFIFOW(fd,2)-2].amount <= 0){
				p->inventory[RFIFOW(fd,2)-2].nameid=0;
			}
			if(nameid==602){//蝶の羽
				mmo_map_changemap(fd,sd,p->save_point.map,p->save_point.x,p->save_point.y,2);
			}else if(nameid==601){//ハエの羽
				srand((unsigned)time(NULL));
				sd->x = (double)(((double)rand()/(double)RAND_MAX) * 300);
				srand((unsigned)time(NULL));
				sd->y = (double)(((double)rand()/(double)RAND_MAX) * 300);
				mmo_map_changemap(fd,sd,sd->mapname,sd->x,sd->y,2);
			}else if(nameid==611){//虫眼鏡
				int i,c;
				WFIFOW(fd,0) = 0x177;
				for(i=c=0;i<100;i++)
					//未鑑定アイテムならば
					if(sd->status.inventory[i].identify != 1)
					{
						WFIFOW(fd,4+c*2) = i+2;//sd->status.inventory[i].nameid;
						c++;
					}
				WFIFOW(fd,2) = 4+c*2;
				WFIFOSET(fd,4+c*2);
			}else if(nameid==604){//古木の枝　　ランダムで敵を召喚
				/*
				int mapno,npc;
				unsigned char buf[256];
				mapno = sd->mapno;
				npc = (double)(((double)rand()/(double)RAND_MAX) * 250) + 1001;
				map_data[sd->mapno].npc[npc]->x = sd->x;
				map_data[sd->mapno].npc[npc]->y = sd->y;
				map_data[mapno].npc[map_data[mapno].npc_num]=malloc(sizeof(struct npc_data));
			    map_data[mapno].npc[map_data[mapno].npc_num]->u.mons.speed=200;
				map_data[mapno].npc[map_data[mapno].npc_num]->class=npc;
				map_data[mapno].npc[map_data[mapno].npc_num]->id=npc;
				map_data[mapno].npc[map_data[mapno].npc_num]->block.subtype=MONS;
				map_data[mapno].npc[map_data[mapno].npc_num]->u.mons.hp=10;
				map_data[mapno].npc[map_data[mapno].npc_num]->u.mons.script = 1;
				memcpy(map_data[mapno].npc[map_data[mapno].npc_num]->name,npc,24);
				respawn_mons2(mapno,map_data[mapno].npc_num);
				map_data[mapno].npc_num++;
				*/
			}else if(nameid==605){//アンティペインメント 　インデュアの効果
				WFIFOW(fd,0) = 0x147;
				WFIFOW(fd,2) = 8;
				WFIFOW(fd,4) = skill_db[8].type_inf;
				WFIFOW(fd,6) = 0;
				WFIFOW(fd,8) = 1;
				WFIFOW(fd,10) = skill_db[8].sp;
				WFIFOW(fd,12) = skill_db[8].range;
				memcpy(WFIFOP(fd,14),"アンティペインメント",24);
				WFIFOB(fd,38) = 0;
				mmo_map_sendarea(fd,WFIFOP(fd,0),39,0);
			}else if(nameid==606){//アロエベラ　　SP全快
				p->sp = p->max_sp;
				WFIFOW(fd,0) = 0xb0;
				WFIFOW(fd,2) = 0007;
				WFIFOL(fd,4) = p->sp;
				WFIFOSET(fd,8);
			}else if(nameid==607){//イグドラシルの実  HP,SP全快
					p->hp = p->max_hp;
					p->sp = p->max_sp;
					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0005;
					WFIFOL(fd,4) = p->hp;
					WFIFOSET(fd,8);
				
					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0007;
					WFIFOL(fd,4) = p->sp;
					WFIFOSET(fd,8);
			}else if(nameid==608){//イグドラシルの種　　HP、SP　半回復
					p->hp += p->max_hp/2;
					p->sp += p->max_sp/2;
					if(p->hp > p->max_hp)p->hp = p->max_hp;
					if(p->sp > p->max_sp)p->sp = p->max_sp;
					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0005;
					WFIFOL(fd,4) = p->hp;
					WFIFOSET(fd,8);
				
					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0007;
					WFIFOL(fd,4) = p->sp;
					WFIFOSET(fd,8);
			}else if(nameid==609){//変魂のお札  未実装
				
			}else if(nameid==610){//イグドラシルの葉　　生き返ってHP1
				WFIFOW(fd,0) = 0x147;
				WFIFOW(fd,2) = 54;
				WFIFOW(fd,4) = skill_db[54].type_inf;
				WFIFOW(fd,6) = 0;
				WFIFOW(fd,8) = 1;
				WFIFOW(fd,10) = skill_db[54].sp;
				WFIFOW(fd,12) = skill_db[54].range;
				memcpy(WFIFOP(fd,14),"イグドラシルの葉",24);
				WFIFOB(fd,38) = 0;
				mmo_map_sendarea(fd,WFIFOP(fd,0),39,0);
			}else if(nameid==603){//青い箱　　ランダムでアイテムを入手
					int i1=502,i2=1,len=0;
					struct item tmp_item;
				i1 = search_item(1);
				if(i1>7000 || (619 <= i1 && i1 <= 643) || 
					(1256 <= i1 && i1<=1261)){i1=644;}//7000番以降は未実装な部分が多く不安定なため、今は未実装としておく
      			memset(&tmp_item,0,sizeof(tmp_item));
      			tmp_item.nameid=i1;
      			tmp_item.amount=i2;
      			tmp_item.identify=1;
      			len=mmo_map_item_get(fd,WFIFOP(fd,0),&tmp_item);
      			if(len>0) WFIFOSET(fd,len);
			}else if(nameid==616){//古いカード帖　　ランダムでカードを入手
					int i1=502,i2=1,len=0;
					struct item tmp_item;
				i1 = search_item(3);
				if(i1>7000 || (619 <= i1 && i1 <= 643) || 
					(1256 <= i1 && i1<=1261)){i1=644;}//7000番以降は未実装な部分が多く不安定なため、今は未実装としておく
      			memset(&tmp_item,0,sizeof(tmp_item));
      			tmp_item.nameid=i1;
      			tmp_item.amount=i2;
      			tmp_item.identify=1;
      			len=mmo_map_item_get(fd,WFIFOP(fd,0),&tmp_item);
      			if(len>0) WFIFOSET(fd,len);
			}else if(nameid==617){//古い紫色の箱   ランダムで装備品を入手
					int i1=502,i2=1,len=0;
					struct item tmp_item;
				i1 = search_item(2);
				if(i1>7000 || (619 <= i1 && i1 <= 643) || 
					(1256 <= i1 && i1<=1261)){i1=644;}//7000番以降は未実装な部分が多く不安定なため、今は未実装としておく
      			memset(&tmp_item,0,sizeof(tmp_item));
      			tmp_item.nameid=i1;
      			tmp_item.amount=i2;
      			tmp_item.identify=1;
      			len=mmo_map_item_get(fd,WFIFOP(fd,0),&tmp_item);
      			if(len>0) WFIFOSET(fd,len);
			}else if(nameid==618){//古い巻物
					int i1=502,i2=1,len=0;
					struct item tmp_item;
				i1 = search_item(3);
				if(i1>7000 || (619 <= i1 && i1 <= 643) || 
					(1256 <= i1 && i1<=1261)){i1=644;}//7000番以降は未実装な部分が多く不安定
      			memset(&tmp_item,0,sizeof(tmp_item));
      			tmp_item.nameid=i1;
      			tmp_item.amount=i2;
      			tmp_item.identify=1;
      			len=mmo_map_item_get(fd,WFIFOP(fd,0),&tmp_item);
      			if(len>0) WFIFOSET(fd,len);
			}else if(nameid==644){//プレゼントボックス ランダムで超貴重品(カード、装備、消耗品)	を入手できる
					int i1=502,i2=1,len=0;
					struct item tmp_item;
				i1 = search_item(4);
				if(i1>7000 || (619 <= i1 && i1 <= 643) || 
					(1256 <= i1 && i1<=1261)){i1=644;}//7000番以降は未実装な部分が多く不安定なため、今は未実装としておく
      			memset(&tmp_item,0,sizeof(tmp_item));
      			tmp_item.nameid=i1;
      			tmp_item.amount=i2;
      			tmp_item.identify=1;
      			len=mmo_map_item_get(fd,WFIFOP(fd,0),&tmp_item);
      			if(len>0) WFIFOSET(fd,len);
			}else if(nameid == 643){// ADDED on 04/09/2003 --------------
				int i,c;
				WFIFOW(fd,0) = 0x1a6;
				for(i=c=0;i<100;i++)
					//タマゴならば
					if((sd->status.inventory[i].nameid >= 9001)&&(sd->status.inventory[i].nameid <=9024))
					{
						WFIFOW(fd,4+c*2) = i+2;//sd->status.inventory[i].nameid;
						c++;
					}
				WFIFOW(fd,2) = 4+c*2;
				WFIFOSET(fd,4+c*2);


				//WFIFOW(fd,0) = 0x1a6;
				//WFIFOW(fd,2) = 6;
				//WFIFOW(fd,4) = RFIFOW(fd,2);
				//WFIFOSET(fd,6);
				//printf("index=%d\n",RFIFOW(fd,2));
				// --------------------------------------------------
			}
     }else{
     	 	if(nameid==714){//エンペリウム　　ギルド作成
     	 		
     	 	}
     }

}
/********************************************************************************************/
