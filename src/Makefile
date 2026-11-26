CC = gcc
CFLAGS = -g -lpq
SRCS = svr.c item.c db.c npc.c skill.c map.c session.c thr_act.c objlist.c
TARGET = svr

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)