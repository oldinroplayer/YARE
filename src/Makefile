CFLAGS = -O9 -Wall -g

all: login2 char2 map2

login2: login2.o core.o
	$(CC) -o $@ $>

char2: char2.o core.o
	$(CC) -o $@ $>

map2: map2.o core.o save.o grfio.o npc.o itemdb.o skill_db.o script.o party.o guild.o pet.o
	$(CC) -o $@ $> -lz

login2.o: login2.c core.h mmo.h
char2.o: char2.c core.h mmo.h char2.h
map2.o: map2.c char2.c login2.c core.h mmo.h grfio.h npc.h itemdb.h save.h map2.h skill_db.h party.h guild.h pet.h
npc.o: npc.c core.h mmo.h npc.h itemdb.h script.h map2.h
itemdb.o: itemdb.c itemdb.h grfio.h itemdb.h
skill_db.o: skill_db.c skill_db.h
core.o: core.c core.h
grfio.o: grfio.c grfio.h
save.o: save.c mmo.h save.h
party.o: party.c party.h mmo.h
guild.o: guild.c guild.h mmo.h
script.o: script.c mmo.h core.h npc.h script.h map2.h
pet.o: pet.c pet.h npc.h

clean:
	rm -f *.o
