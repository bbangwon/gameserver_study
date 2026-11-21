CC = gcc
CFLAGS = -g -lpq
SRCS = svr.c session.c thr_act.c objlist.c packet.c que.c map.c db.c
TARGET = svr

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)