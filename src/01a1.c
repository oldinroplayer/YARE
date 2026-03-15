{
	int type = RFIFOB(fd,2);

	if(type == 0)
	{
		WFIFOW(fd,0) = 0x1a2;
		memcpy(WFIFOP(fd,2),sd->status.pet.pet_name,24);
		WFIFOB(fd,26) = sd->status.pet.pet_name_flag;
		WFIFOW(fd,27) = sd->status.pet.pet_level;
		WFIFOW(fd,29) = sd->status.pet.pet_hungry;;
		WFIFOW(fd,31) = sd->status.pet.pet_friendly;
		WFIFOW(fd,33) = sd->status.pet.pet_accessory;
		WFIFOSET(fd,35);
	}
	else if(type == 1)
	{
		sd->status.pet.pet_hungry += 80;
		if(sd->status.pet.pet_hungry > 100)
			sd->status.pet.pet_hungry = 100;
		WFIFOW(fd,0) = 0x1a4;
		WFIFOB(fd,2) = 2;
		WFIFOL(fd,3) = sd->status.pet.pet_id_as_npc;
		WFIFOL(fd,7) = sd->status.pet.pet_hungry;
		WFIFOSET(fd,11);

		sd->status.pet.pet_friendly += 20;
		if(sd->status.pet.pet_friendly > 200)
			sd->status.pet.pet_friendly = 200;
		WFIFOW(fd,0) = 0x1a4;
		WFIFOB(fd,2) = 1;
		WFIFOL(fd,3) = sd->status.pet.pet_id_as_npc;
		WFIFOL(fd,7) = sd->status.pet.pet_friendly;
		WFIFOSET(fd,11);

		WFIFOW(fd,0) = 0x1a3;
		WFIFOB(fd,2) = 0x1;
		WFIFOB(fd,3) = 0xd;
		WFIFOB(fd,4) = 0x2;
		WFIFOSET(fd,5);
	}
	else if(type == 2)
	{
		WFIFOW(fd,0) = 0x1a4;
		WFIFOB(fd,2) = 4;
		WFIFOL(fd,3) = sd->status.pet.pet_id_as_npc;
		WFIFOL(fd,7) = 2;
		WFIFOSET(fd,11);
	}
	else if(type == 3)
	{
		int i,nameid;

		//printf("1a1 type3 reached!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

		switch(sd->status.pet.pet_class)
		{
			case 1002: nameid=9001; break;
			case 1113: nameid=9002; break;
			case 1031: nameid=9003; break;
			case 1063: nameid=9004; break;
			case 1049: nameid=9005; break;
			case 1011: nameid=9006; break;
			case 1042: nameid=9007; break;
			case 1035: nameid=9008; break;
			case 1067: nameid=9009; break;
			case 1107: nameid=9010; break;
			case 1052: nameid=9011; break;
			case 1014: nameid=9012; break;
			case 1077: nameid=9013; break;
			case 1019: nameid=9014; break;
			case 1056: nameid=9015; break;
			case 1057: nameid=9016; break;
			case 1023: nameid=9017; break;
			case 1026: nameid=9018; break;
			case 1110: nameid=9019; break;
			case 1170: nameid=9020; break;
			case 1029: nameid=9021; break;
			case 1156: nameid=9022; break;
			case 1109: nameid=9023; break;
			case 1101: nameid=9024; break;
			default: nameid = -1; break;
		}


		for(i=0;i<100;i++)
		if(sd->status.inventory[i].nameid == nameid)
		{
			++p->inventory[i].amount;
			break;
		}


		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = sd->status.pet.pet_id_as_npc;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd,7);

		sd->status.pet.activity = 0;

		del_block(&map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block);


	}
	else if(type == 4)
	{
		WFIFOW(fd,0) = 0x1a4;
		WFIFOB(fd,2) = 3;
		WFIFOL(fd,3) = sd->status.pet.pet_id_as_npc;
		WFIFOL(fd,7) = 0;
		WFIFOSET(fd,11);

		sd->status.pet.pet_accessory = 0;
	}

}
