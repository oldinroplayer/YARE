/********************** Command Functions *************************/
/**************Translated from original Japanese version***********/
/****************************Translation by  Angler****************/
{
/**************   Check if the item ID that was asked for exists **********************/
int is_item(int object){
	char str[80];
	FILE *fp;
	int item_id=0,result=0;
	
	if((fp = fopen("item_all.list","r"))==NULL){
		printf("Open the item list!\n");
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
      	char *message;
      	message = strchr(RFIFOP(fd,4)+strlen(sd->status.name),'!');
if((int*)message!=NULL){//メッセージ中に @ が現れたら
	  	char comand[40];	//@rura などのシステムコマンド文字列が　入る
	  	char temp0[40];		//引数1個目
//	  	char temp1[40];		//引数2個目
//	  	char temp2[40];		//引数3個目
	  	char moji[200];		//長文文字列
      	int  x=0,y=0,z=0;	//座標とか
      	int i;
   		
		struct mmo_charstatus *p;
   		sscanf(message,"%s",comand);
  		p=&sd->status;
//ルーラ
//「!rura or !map マップファイル名 ｘ座標 y座標」として、打つ。
	  sscanf(message,"%s%s%d%d",comand,temp0,&x,&y);
      if( (strncmp(comand,"!rura",5)==0) || (strncmp(comand,"!map",4)==0))  {
      	if(x>0 && x<325 && y>0 && y<325){
					char moji[200];
      	    for(i=0;map[i][0];i++){
    			if(strcmp(map[i],temp0)==0){
					char moji[200];
          			mmo_map_changemap(fd,sd,temp0,x,y,2);
					strcpy(moji,"Warped!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
          			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
    				}
				}
					strcpy(moji,"そんなマップファイル名は存在しません。\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
          			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
      	}
	}


//現在位置を含め、冒険の記録をセーブする
//「@save」と入力
		sscanf(message,"%s",comand);
		if(strncmp(comand,"!save",5)==0){
					char moji[200];
			strcpy(p->last_point.map,sd->mapname);
			strcpy(p->save_point.map,sd->mapname);
			p->last_point.x = sd->x;
			p->save_point.x = sd->x;
			p->last_point.y = sd->y;
			p->save_point.y = sd->y;
			mmo_char_save(sd);
					strcpy(moji,"Location saved\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
		}

//セーブ地点にワープする
//「@load」と入力
		sscanf(message,"%s",comand);
		if(strncmp(comand,"!load",5)==0){
					char moji[200];
			strcpy(sd->mapname,p->last_point.map);
			//strcpy(p->save_point.map,sd->mapname);
			sd->x = p->save_point.x;
			sd->y = p->save_point.y;
			mmo_map_changemap(fd,sd,sd->mapname,sd->x,sd->y,2);
					strcpy(moji,"Returning to saved location...\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
		}

//歩行スピード変更
//「@speed スピード値[1～1000]」と入力,小さいほど速い、めちゃ挙動不審ｗ
		sscanf(message,"%s%d",comand,&x);
		if(strncmp(comand,"!speed",6)==0){
			if(x>0 && x<1000){
					char moji[200];
				sd->speed=x;
				sd->walktimer=x;
					strcpy(moji,"Speed change!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
			}
		}

//状態変更
//「@option 1 20 15」のように値を記述
/*
	R 0119 <ID>.l <param1>.w <param2>.w <param3>.w ?.B
	見た目変更
	param1=01 石化？(固まる)
	param1=02 フロストダイバで凍り漬け?
	param1=03 ぴよる
	param1=04 眠り
	param1=06 暗闇(歩ける)
	
	param2=01 毒
	param2=02 背後霊
	param2=04 沈黙状態
	param2=16 周囲を暗くする
	
	param3=01 サイトかルワッチ?
	param3=02 ハイディング状態?
	param3=04 クローキング状態?
	param3=08 カート付き
	param3=16 鷹付き
	param3=32 ペコペコ乗り
	param3=64 消える
*/
		sscanf(message,"%s%d%d%d",comand,&x,&y,&z);
		if(strncmp(comand,"!option",7)==0){
					char moji[200];
			/*
			p->option は次のようなbitにおいて成り立っている？
			pram1	0000 0000 0000 0000
			pram2	0000 0000 0000 0000
			pram3	0000 0000 0000 0000 
			
			*/
			
			WFIFOW(fd,0)=0x0119;
			WFIFOL(fd,2)=sd->account_id;
			WFIFOW(fd,6)=x;
			WFIFOW(fd,8)=y;
			WFIFOW(fd,10)=z;
			mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
			
			//x = x<<16;
			//y = y<<8;
			p->option=x|y|z;
			
					strcpy(moji,"Lots of changes!!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
		}
// !hide 
sscanf(message,"%s",comand); 
  if(strncmp(comand,"!hide",7)==0)
  {
    WFIFOW(fd,0)=0x0119;
    WFIFOL(fd,2)=sd->account_id;  
    WFIFOW(fd,6)=00;
    WFIFOW(fd,8)=00;
  if(WFIFOW(fd,10)==04)
    WFIFOW(fd,10)=00;
    else 
    WFIFOW(fd,10)=04;
    mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
    goto idou;
  }
//その場で転職
//「@jobchange 職業ID[0～12]」のように職業ごとに値を記述
/*
０がノービス
１が剣士
２がまじしゃん
３があーちゃー
４がｱｺライト
５が商人
６が盗賊
７が騎士
８がプリースト
９がウィザード
１０ブラックスミス
１１がハンター
１２がアサシン

１３
*/
		sscanf(message,"%s%d",comand,&x);
		if( (strncmp(comand,"!jobchange",10)==0) || (strncmp(comand,"!job",4)==0)){
			if(x>=0 && x<=12){
					char moji[200];
			mmo_map_jobchange(fd,x);
					strcpy(moji,"Job changed!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
			}
		}

//自殺
//「@die」と入力する。
		sscanf(message,"%s",comand);
		if(strncmp(comand,"!die",4)==0){
					char moji[200];
			p->hp = 0;
			WFIFOW(fd,0) = 0xb0;
			WFIFOW(fd,2) = 0005;
			WFIFOL(fd,4) = p->hp;
			WFIFOSET(fd,8);
			WFIFOW(fd,0) = 0x80;
            WFIFOL(fd,2) = sd->account_id;
            WFIFOB(fd,6) =  1;
			mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
					strcpy(moji,"T-Bone!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
		}

//生き返り
//[@alive]と入力する
		sscanf(message,"%s",comand);
		if(strncmp(comand,"!alive",6)==0){
					char moji[200];
					p->hp = p->max_hp;
					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0005;
					WFIFOL(fd,4) = p->hp;
					WFIFOSET(fd,8);
				
					WFIFOW(fd,0) = 0x148;
					WFIFOL(fd,2) = sd->account_id;
					mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x148], 0 );
					strcpy(moji,"Life! We have LIFE!!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
		}

//天の声
//「@kami 台詞」と入力
      		sscanf(message,"%s%s",comand,moji);
		if(strncmp(comand,"!kami",5)==0){
			x=0;
			x=strlen(message)+4;
      WFIFOW(fd,0)=0x9a;
      WFIFOW(fd,2)=x;
      strcpy(WFIFOP(fd,4),moji);
      mmo_map_sendall(fd, WFIFOP(fd,0), WFIFOW(fd,2), 0 );
      goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
      }

//HP,SP 更新
//「@heal 数値(HP) 数値(SP)」と入力
//省略して「@heal」とだけ打てば、両方全回復
		x=y=0;
      		sscanf(message,"%s%d%d",comand,&x,&y);
		if(strncmp(comand,"!heal",5)==0){
					char moji[200];
			if(x==0 && y==0){//省略記述で全回復
				WFIFOW(fd,0) = 0x13d;
				WFIFOW(fd,2) = 5;
				WFIFOW(fd,4) = p->max_hp - p->hp;
				WFIFOSET(fd,6);
				
				WFIFOW(fd,0) = 0x13d;
				WFIFOW(fd,2) = 7;
				WFIFOW(fd,4) = p->max_sp - p->sp;
				WFIFOSET(fd,6);
				p->hp = p->max_hp+1;
				p->sp = p->max_sp+1;
				
			}else if(x >= -10000 && x <= 10000 && y>= -10000 && y <= 10000){
				p->hp += x;
				p->sp += y;
				WFIFOW(fd,0) = 0x13d;
				WFIFOW(fd,2) = 5;
				WFIFOW(fd,4) = x;
				WFIFOSET(fd,6);
				
				WFIFOW(fd,0) = 0x13d;
				WFIFOW(fd,2) = 7;
				WFIFOW(fd,4) = y;
				WFIFOSET(fd,6);
			}
			//HP,SPが最大値をオーバーしていたら、最大値でとめる。
					if(p->hp > p->max_hp || p->hp < 0){
					p->hp = p->max_hp;
					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0005;
					WFIFOL(fd,4) = p->hp;
					WFIFOSET(fd,8);
					}
					
					if(p->sp > p->max_sp || p->sp < 0){
					p->sp = p->max_sp;
					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0007;
					WFIFOL(fd,4) = p->sp;
					WFIFOSET(fd,8);
					}

					strcpy(moji,"HP and SP recovered.\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
					goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
      }


//アイテムゲット
//「@item アイテムのID 個数」と入力
			x=501;y=1;
      		sscanf(message,"%s%d%d",comand,&x,&y);
			if(strncmp(comand,"!item",5)==0){
				struct item tmp_item;
				
				if(is_item(x)==1){
					char moji[200];
      			memset(&tmp_item,0,sizeof(tmp_item));
      			tmp_item.nameid=x;
      			tmp_item.amount=y;
      			tmp_item.identify=1;
      			len=mmo_map_item_get(fd,WFIFOP(fd,0),&tmp_item);
      			if(len>0) WFIFOSET(fd,len);
      			
					strcpy(moji,"Got item!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			      goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
			    }else{
					char moji[200];
					strcpy(moji,"No item with that ID exists!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			    	goto idou;
			    }
      }

//勝手に実装@popee
//Lvアップコマンド「@Lvup」と入力
			sscanf(message,"%s%d",comand,&x);
			if(strncmp(comand,"!base_up",8)==0){
				if(p->base_level + x <= 200){	
					char moji[200];
					mmo_map_level_up(fd,x);	
					strcpy(moji,"Level UPPPPPPP~~~\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
				goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
				}
				else
				{
					strcpy(moji,"Can't go higher than this!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
					goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
				}
			}

//Lvアップコマンド「@Lvup」と入-ﾍ
			sscanf(message,"%s%d",comand,&x);
			if(strncmp(comand,"!job_up",7)==0){
				if(p->job_level + x <= 50){	
					char moji[200];
					mmo_map_job_lv_up(fd,x);	
					strcpy(moji,"Your Job lebel has been raised.\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
				goto idou;
				}
				else
				{
					strcpy(moji,"Job Lvl 50 is maximum\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
					goto idou;
				}
			}



//ヘルプコマンド　＠コマンドの全容を説明してくれる。
//「@h」と入力
			sscanf(message,"%s",comand);
			if(strncmp(comand,"!h",2)==0 || strncmp(comand,"!help",5)==0 ){
					char moji[400];
					FILE *file;
					file = fopen("config/help.txt","r");
					
					sprintf(moji,"Help Commands\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
					while(fgets(moji,380,file)!=NULL){
						strcat(moji," \n");
						WFIFOW(fd,0) = 0x8e;
						WFIFOW(fd,2) = 4+strlen(moji);
						memcpy(WFIFOP(fd,4),moji,strlen(moji));
						WFIFOSET(fd,4 + strlen(moji));
					}
					fclose(file);
				goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
			}

//GM衣装になる！　まだ完全動作しません。一度サーバーを落として再ログインする必要があります。
//「@GM」と入力
/*
	R 006b <len>.w <charactor select data>.106B*
	キャラセレ鯖接続成功&キャラクタデータ
	<charactor select data> = <charactor ID>.l <base exp>.l <zeny>.l <job exp>.l <job level>.l ?.8B <option>.l <karma>.l <manner>.l ?.2B <HP>.w <MaxHP>.w <SP>.w <MaxSP>.w <speed>.w <class>.w <hair>.w ?.2w <base level>.w <skill point>.w ?.w <sheild>.w <head option top>.w <head option mid>.w <hair color>.w ?.w <name>.24B <STR>.B <AGI>.B <VIT>.B <INT>.B <DEX>.B <LUK>.B <charactor number>.B ?.B
*/
			sscanf(message,"%s",comand);
			if(strncmp(comand,"!GM",3)==0){
					char moji[200];
				int next_id;
				next_id = mmo_char_GM(sd);
				WFIFOW(char_fd,0)=0x3001;
				WFIFOL(char_fd,2)=fd;
				WFIFOSET(char_fd,6);
				sd->account_id=next_id;
				sd->status.account_id=next_id;
					strcpy(moji,"Want to be a game master?\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
				goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
			}
//PVP解除　　ただ、マップ移動しないと解除されません。
//「@pvpoff」と入力
			sscanf(message,"%s",comand);
			if(strncmp(comand,"!pvpoff",7) == 0){
					char moji[200];
				WFIFOW(fd,0)=0x199;
				WFIFOW(fd,2)=1;
				WFIFOSET(fd,4);
					strcpy(moji,"Long enough, don't you think?\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
			}
//PVP実装
//「@pvp」と入力
			sscanf(message,"%s",comand);
			if(strncmp(comand,"!pvp",4)==0){
					char moji[200];
				WFIFOW(fd,0)=0x199;
				WFIFOW(fd,2)=1;
				WFIFOSET(fd,4);
			//	mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x199], 0 );
				WFIFOW(fd,0) = 0x19a;
				WFIFOL(fd,2) = sd->account_id;
				WFIFOL(fd,6) = rank;
				WFIFOL(fd,10) = users_global;
				mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x19a], 0 );
					strcpy(moji,"Kill Kill Kill Kill Kill Kill!!!\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
				goto idou;
			}


//髪型、髪の色、服の色、変更
//「@model x y z」のように値を入力、ただしこれらを変更した状態で2HQを使うとエラーが・・・(´Д｀)　解明しだい直します。
/*	例：@model 15 4 0

x [0～17]髪型
y [0～8]髪の色
z [0～4]服の色

*/
			sscanf(message,"%s%d%d%d",comand,&x,&y,&z);
			if(strncmp(comand,"!model",6)==0){
			char moji[200];
				if(x >= 0 && x < 18 && y >=0 && y < 9 && z >= 0 && z <= 4){
	            //服の色変更
	            if (sd->status.class == 12){
	                //アサシンは服の色未実装
	                	strcpy(moji,"Assassin isn't working yet...\n");
						WFIFOW(fd,0) = 0x8e;
						WFIFOW(fd,2) = 4 + strlen(moji);
						memcpy(WFIFOP(fd,4),moji,strlen(moji));
						WFIFOSET(fd,4 + strlen(moji));
	              	}else{
	            	sd->status.hair = x;
	            	sd->status.hair_color = y;
	            	sd->status.clothes_color = z;
	            	
	            	WFIFOW(fd,0) = 0xc3;
	            	WFIFOL(fd,2) = sd->account_id;
	            	WFIFOB(fd,6) = 1;
	            	WFIFOB(fd,7) = x;
	            	WFIFOSET(fd,8);
	            	
	            	WFIFOW(fd,0) = 0xc3;
	            	WFIFOL(fd,2) = sd->account_id;
	            	WFIFOB(fd,6) = 6;
	            	WFIFOB(fd,7) = y;
	            	WFIFOSET(fd,8);

	            	WFIFOW(fd,0) = 0xc3;
	            	WFIFOL(fd,2) = sd->account_id;
	            	WFIFOB(fd,6) = 7;
	            	WFIFOB(fd,7) = z;
	            	WFIFOSET(fd,8);
	            }
						strcpy(moji,"Amazing!\n");
						WFIFOW(fd,0) = 0x8e;
						WFIFOW(fd,2) = 4 + strlen(moji);
						memcpy(WFIFOP(fd,4),moji,strlen(moji));
						WFIFOSET(fd,4 + strlen(moji));
					goto idou;
				}else{
						strcpy(moji,"Invalid values. Cancelled!\n");
						WFIFOW(fd,0) = 0x8e;
						WFIFOW(fd,2) = 4 + strlen(moji);
						memcpy(WFIFOP(fd,4),moji,strlen(moji));
						WFIFOSET(fd,4 + strlen(moji));
				}
			}










//テストコマンド デバッグ用なんです。気にしないで下さい。
//「@test」と入力
//			R 00fb <len>.w <party name>.24B {<ID>.l <nick>.24B <map name>.16B <leader>.B <offline>.B}.46B*
			sscanf(message,"%s",comand);
			if(strncmp(comand,"!test",5)==0){
				WFIFOW(fd,0) = 0xfb;
				WFIFOW(fd,2) = 74;
				strcpy(WFIFOP(fd,4),"Horseman group");
				WFIFOL(fd,28) = 100006;
				strcpy(WFIFOP(fd,32),"Applying~");
				strcpy(WFIFOP(fd,56),"Where?");
				WFIFOB(fd,72)=1;
				WFIFOB(fd,73)=1;
				WFIFOSET(fd,74);
				goto idou;
			}


		sscanf(message,"%s",comand);
		if(strncmp(comand,"!reload_config",5)==0){
			reload_config();
			goto idou;
		}




//テストコマンド デバッグ用なんです。気にしないで下さい。
//「@test」と入力
/*
R 0196 <type>.w <ID>.l
	(主に?)プリ関係の増強もの色々。IDはtargetと思われるが自分相手のみしか来ない?
	type=02 IMPOSITIO付与
	type=03 IMPOSITIO解除
	type=07 ASPERSIO付与
	type=08 ASPERSIO解除
	type=0b KYRIE付与
	type=0c KYRIE解除
*/
			sscanf(message,"%s%d",comand,&x);
			if(strncmp(comand,"!debug",6)==0){
				WFIFOW(fd,0) = 0x196;
				WFIFOW(fd,2) = x;
				WFIFOL(fd,4) = sd->account_id;
				WFIFOSET(fd,8);
				goto idou;
			}
			

// ADDED
// ルーラ簡易版
//「@go 数字」として、打つ。
/*
	0=プロンテラ
	1=モロク
	2=ゲフェン
	3=フェイヨン
	4=アルベルタ
	5=イズ
	6=アルデバラン
	7=ルティエ
*/
			sscanf(message,"%s%d",comand,&x);
			if(strncmp(comand,"!go",3)==0)
			{
      			if(x==0)
      			{
             		mmo_map_changemap(fd,sd,"prontera.gat",156,191,2);
          			goto idou;
				}
				else if(x==1)
				{
             		mmo_map_changemap(fd,sd,"morocc.gat",156,96,2);
          			goto idou;
				}
				else if(x==2)
				{
             		mmo_map_changemap(fd,sd,"geffen.gat",119,60,2);
          			goto idou;
				}
				else if(x==3)
				{
             		mmo_map_changemap(fd,sd,"payon.gat",89,122,2);
          			goto idou;
				}
				else if(x==4)
				{
             		mmo_map_changemap(fd,sd,"alberta.gat",192,147,2);
          			goto idou;
				}
				else if(x==5)
				{
             		mmo_map_changemap(fd,sd,"izlude.gat",128,114,2);
          			goto idou;
				}
				else if(x==6)
				{
             		mmo_map_changemap(fd,sd,"aldebaran.gat",140,131,2);
          			goto idou;
				}
				else if(x==7)
				{
             		mmo_map_changemap(fd,sd,"xmas.gat",147,134,2);
          			goto idou;
				}
				else
				{
					char moji[200];
					strcpy(moji,"Error in map destination...\n");
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 4 + strlen(moji);
					memcpy(WFIFOP(fd,4),moji,strlen(moji));
					WFIFOSET(fd,4 + strlen(moji));
          			goto idou;//他の人へのメッセージ表示を　全て飛ばして、処理終了
				}
			}



	}
}
/************************************************************************************/
