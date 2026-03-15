CFLAGS = -O9 -Wall -g -fcommon
LDFLAGS = -Wl,--allow-multiple-definition

.PHONY: all clean login-server char-server map-server

all: login-server char-server map-server

login-server: login2.o core.o
	$(CC) -o $@ $^ $(LDFLAGS)

char-server: char2.o core.o
	$(CC) -o $@ $^ $(LDFLAGS)

map-server: map2.o core.o save.o grfio.o npc.o itemdb.o skill_db.o script.o party.o guild.o pet.o
	$(CC) -o $@ $^ -lz $(LDFLAGS)

login2.o: login2.c core.h mmo.h
char2.o: char2.c core.h mmo.h char2.h
map2.o: map2.c char2.c login2.c core.h mmo.h grfio.h npc.h itemdb.h save.h map2.h skill_db.h party.h guild.h pet.h
npc.o: npc.c core.h mmo.h npc.h itemdb.h script.h map2.h pet.h
itemdb.o: itemdb.c itemdb.h grfio.h itemdb.h
skill_db.o: skill_db.c skill_db.h
core.o: core.c core.h
grfio.o: grfio.c grfio.h
save.o: save.c mmo.h save.h
party.o: party.c party.h mmo.h
guild.o: guild.c guild.h mmo.h
script.o: script.c mmo.h core.h npc.h script.h map2.h
pet.o: pet.c pet.h npc.h mmo.h

clean:
	rm -f *.o
	rm -f *.exe
