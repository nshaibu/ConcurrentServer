# -*-Makefile -*-

CC=gcc
LIBS=-lpthread
MYSQL_LIBS=`mysql_config --cflags --libs`
OBJS=addr_table.o packets.o queue.o thread_info_block.o server.o

main: ./main.c $(OBJS)
	$(CC) -g -o main main.c $(OBJS) $(LIBS) $(MYSQL_LIBS)

server.o: ./server.c
	$(CC) -g -c ./server.c $(MYSQL_LIBS)

queue.o: ./libs/queue.c
	$(CC) -g -c ./libs/queue.c

iterator.o: ./libs/iterator.c
	$(CC) -g -c ./libs/iterator.c

thread_info_block.o: ./data_structs/thread_info_block.c
	$(CC) -g -c ./data_structs/thread_info_block.c $(MYSQL_LIBS)

packets.o: ./data_structs/packets.c
	$(CC) -g -c ./data_structs/packets.c

addr_table.o: ./data_structs/addr_table.c
	$(CC) -g -c ./data_structs/addr_table.c
	
clean:
	rm -rf *.o