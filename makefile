all: crc crs crm

crc:
	gcc chatroom_client.c -o client.o -lpthread

crs:
	gcc chatroom_server.c -o server.o -lpthread

crm:
	gcc chatroom_manager.c -o manager.o -lpthread

clean:
	rm *.o
