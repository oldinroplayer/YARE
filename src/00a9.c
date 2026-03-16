//
// PvPGN YARE MOD V1.0 (Yeat Another Ragnarok Emulator) - (Server)
// Copyright (c) Project-YARE & PvPGN 2003
// www.project-yare.com
// forum.project-yare.net
// www.pvpgn.org
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// pet system preparation
if((sd->status.inventory[RFIFOW(fd,2)-2].nameid >= 10001)&&(sd->status.inventory[RFIFOW(fd,2)-2].nameid <= 10019))
{
	if(sd->status.pet.activity == 1)
	{
		int equip_able = 0;

		switch(sd->status.inventory[RFIFOW(fd,2)-2].nameid)
		{
			case 10001: if(sd->status.pet.pet_class==1101){equip_able=1;} break;
			case 10002: if((sd->status.pet.pet_class==1011)||(sd->status.pet.pet_class==1042)){equip_able=1;} break;
			case 10003: if(sd->status.pet.pet_class==1107){equip_able=1;} break;
			case 10004: if(sd->status.pet.pet_class==1109){equip_able=1;} break;
			case 10005: if(sd->status.pet.pet_class==1110){equip_able=1;} break;
			case 10006: if(sd->status.pet.pet_class==1029){equip_able=1;} break;
			case 10007: if(sd->status.pet.pet_class==1063){equip_able=1;} break;
			case 10008: if(sd->status.pet.pet_class==1026){equip_able=1;} break;
			case 10009: if(sd->status.pet.pet_class==1023){equip_able=1;} break;
			case 10010: if(sd->status.pet.pet_class==1019){equip_able=1;} break;
			case 10011: if(sd->status.pet.pet_class==1156){equip_able=1;} break;
			case 10012: if(sd->status.pet.pet_class==1049){equip_able=1;} break;
			case 10013: if((sd->status.pet.pet_class==1002)||(sd->status.pet.pet_class==1113)||(sd->status.pet.pet_class==1031)){equip_able=1;} break;
			case 10014: if(sd->status.pet.pet_class==1052){equip_able=1;} break;
			case 10015: if(sd->status.pet.pet_class==1067){equip_able=1;} break;
			case 10016: if(sd->status.pet.pet_class==1170){equip_able=1;} break;
			case 10017: if((sd->status.pet.pet_class==1014)||(sd->status.pet.pet_class==1077)){equip_able=1;} break;
			case 10018: if(sd->status.pet.pet_class==1057){equip_able=1;} break;
			case 10019: if(sd->status.pet.pet_class==1056){equip_able=1;} break;
			default: equip_able = 0; break;
		}

		if(equip_able == 1)
		{
			WFIFOW(fd,0) = 0x1a4;
			WFIFOB(fd,2) = 3;
			WFIFOL(fd,3) = sd->status.pet.pet_id_as_npc;
			WFIFOL(fd,7) = sd->status.inventory[RFIFOW(fd,2)-2].nameid;
			WFIFOSET(fd,11);

			sd->status.pet.pet_accessory = sd->status.inventory[RFIFOW(fd,2)-2].nameid;
		}

		break;
	}
}

