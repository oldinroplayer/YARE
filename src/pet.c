#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#include "core.h"
#include "mmo.h"
#include "npc.h"
#include "pet.h"

static int *init_npc_id;

int pet_init(struct map_session_data *sd)
{
	int *id,i;

	if(sd->status.pet.pet_id_as_npc == -1)
	{
		id = return_npc_current_id();
		if((*id - *init_npc_id) < MAX_NPC_PER_MAP)
		{
			sd->status.pet.pet_id_as_npc = *id;
			(*id)++;
			// pet_npc_id_on_mapは初期段階では-1
			// （マップ上にまだ一度も表示されていないの意味)
			// （マップが変わればまた-1にする)
			for(i=0;i<MAX_MAP_PER_SERVER;i++)
				sd->status.pet.pet_npc_id_on_map[i] = -1;
		}
		else
		{
			// もしNPC用idが確保できなかった場合は
			// pet_id_as_npc = -2
			sd->status.pet.pet_id_as_npc = -2;
			printf("Could not give npc id for pet\n");
		}
		sd->status.pet.activity = 0;
	}

	return 0;
}

int pet_store_init_npc_id(int *id)
{
	init_npc_id = id;
	printf("INITIAL ID=%d\n",*init_npc_id);

	return 0;
}

