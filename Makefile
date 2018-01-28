# -*-Makefile -*-

CC=gcc
LIBS=-lpthread
MYSQL_LIBS=`mysql_config --cflags --libs`
DEFS=-D_REENTRANT
OPTS=-g -Wall #-Werror
OBJS=addr_table.o packets.o queue.o thread_info_block.o server.o errno_logs.o

main: ./main.c $(OBJS)
	$(CC) $(DEFS) $(OPTS) -o main main.c $(OBJS) $(LIBS) $(MYSQL_LIBS)

server.o: ./server.c
	$(CC) $(DEFS) $(OPTS) -c ./server.c $(MYSQL_LIBS)

queue.o: ./libs/queue.c
	$(CC) $(DEFS) $(OPTS) -c ./libs/queue.c

iterator.o: ./libs/iterator.c
	$(CC) $(DEFS) $(OPTS) -c ./libs/iterator.c

thread_info_block.o: ./data_structs/thread_info_block.c
	$(CC) $(DEFS) $(OPTS) -c ./data_structs/thread_info_block.c $(MYSQL_LIBS)

packets.o: ./data_structs/packets.c
	$(CC) $(DEFS) $(OPTS) -c ./data_structs/packets.c

addr_table.o: ./data_structs/addr_table.c
	$(CC) $(DEFS) $(OPTS) -c ./data_structs/addr_table.c

errno_logs.o: ./error_logs/errno_logs.c
	$(CC) $(DEFS) $(OPTS) -c ./error_logs/errno_logs.c
	
clean:
	rm -rf *.o
