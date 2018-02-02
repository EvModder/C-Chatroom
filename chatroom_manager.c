//Team: Nathaniel Leake 424003778, Chase Elander ###00####
//To compile: "make" or "gcc chatroom_server.c";
//Server available at chatroom-438.cf, or you can run your own

#include "interface.h"
#include <netinet/in.h>
#include <stdlib.h>

int REPLY_SIZE = sizeof(struct Reply);
int NEXT_PORT = 59254;

// room_count = # of active rooms, open_slot = 1st available
static unsigned int room_count = 0, open_slot = 0;

// Chatrooms currently available to join
typedef struct{
	char name[MAX_DATA];
	char host[MAX_DATA];
	int port;
	int num_members;
} chatroom_t;

chatroom_t* chatrooms[MAX_ROOMS];

int add_room(chatroom_t* new_room){
	if(++room_count == MAX_ROOMS){
		--room_count;
		return 0;
	}
	int i;
	for(i=open_slot; i<MAX_ROOMS; ++i){
		if(!chatrooms[i]){
			chatrooms[i] = new_room;
			open_slot = i+1;
			return 1;
		}
	}
	--room_count;
	return 0;
}
int del_room(char* name){
	int i=0;
	for(; i<MAX_ROOMS; ++i){
		if(chatrooms[i] != NULL && strcmp(chatrooms[i]->name, name) == 0){
			// Open this space for a new chatroom
			free(chatrooms[i]);
			chatrooms[i] = NULL;
			if(i < open_slot) open_slot = i;
			--room_count;
			return 1;
		}
	}
	return 0;
}
chatroom_t* get_room(char* name){
	int i = 0;
	for(; i<MAX_ROOMS; ++i){
		if(chatrooms[i] != NULL && strcmp(chatrooms[i]->name, name) == 0){
			return chatrooms[i];
		}
	}
	return NULL;
}

/*
 * Handle a command from a client and store the result in the
 * provided Reply struct
 * @parameter command  command from client to be handled
 * @parameter reply    struct to store the result
 */
void run_command(char* command, struct Reply* reply){
	reply->status = SUCCESS;

	// TODO: comment out debug lines for final submission
	printf("Received: %s\n", command);

	// TODO: Implement these
	if(startswith(command, "LIST")){
		// reply->list_room()
		string reply_temp = SUCCESS + " 0 0 ";

		int i = 0;
		for(; i<MAX_ROOMS; ++i){
			if(chatrooms[i] != NULL){
				reply_temp = reply_temp + chatrooms[i].name + ",";
			}
		}
		strcpy(reply->list_room, reply_temp);
	}
	else if(startswith(command, "JOIN ")){//substr(5) to get name
		chatroom_t* room = get_room(command+5);
		if(room){
			reply->num_member = ++(room->num_members);
			strcpy(reply->host, room->host);
			reply->port = room->port;
		}
		else{
			reply->status = FAILURE_NOT_EXISTS;
		}
		// if(chatrooms.count(room_name)){
		// 	reply->num_member = 
		// }
		// else{
		// 	reply->status = FAILURE_NOT_EXISTS;
		// }
	}
	else if(startswith(command, "CREATE ")){//substr(7) to get name
		if(get_room(command+7)){
			reply->status = FAILURE_ALREADY_EXISTS;
		}
		else{
			// Create a new chatroom and add it to the list
			chatroom_t* room = (chatroom_t*) malloc(sizeof(chatroom_t));
			strcpy(room->name, command+7);
			strcpy(room->host, "uhh stretch goals not reached");
			room->port = ++NEXT_PORT;
			room->num_members = 0;
			add_room(room);

			// Open the new chatroom as a separate process
			char str[12];
			sprintf(str, "%d", room->port);
			char *argv[]={"server.o", str, NULL};
			// printf("Starting chatroom on port %d\n", port);
			if(fork() == 0) execv("./server.o", argv);
		}
	}
	else if(startswith(command, "DELETE ")){
		if(del_room(command+7));
		else reply->status = FAILURE_NOT_EXISTS;
	}
	else{
		reply->status = FAILURE_INVALID;
	}
}

// This is the worker thread for a client (used to initialize pthreads)
void* client_response_worker(int cli_sock){
	// Receive command from client with 5 second timeout
	struct timeval tv = {5, 0};
	setsockopt(cli_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	char command[MAX_DATA];
	if(read(cli_sock, command, MAX_DATA) > 0){
		struct Reply reply;
		run_command(command, &reply);
		write(cli_sock, &reply, REPLY_SIZE);
	}
	else perror("No command from client (or invalid)");
	return NULL;
}

int main(int argc, char** argv){
	//If not specified, use this port as default.
	unsigned short port = argc > 1 ? atoi(argv[1]) : NEXT_PORT;

	// Structures used for setting up the server
	int serv_sock, cli_sock, addr_sz;
	struct sockaddr_in serv_addr, cli_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	addr_sz = sizeof(serv_addr);

	// Open socket, bind it to a port, and listen for connections
	// If any of these fails, we terminate with an error message.
	if((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) <= 0 ||
		bind(serv_sock, (struct sockaddr*) &serv_addr, addr_sz) != 0 || 
		listen(serv_sock, 5))
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}

	// Zero out the chatrooms array
	int i = 0;
	for(; i<MAX_ROOMS; ++i) chatrooms[i] = NULL;

	printf("Server started on port: %d\n", port);
	printf("Ready to begin handling commands\n");

	pthread_t tid;
	while(1){
		// Whenever a client connects, create a new thread to handle its COMMAND
		if((cli_sock = accept(serv_sock, (struct sockaddr*) &cli_addr, &addr_sz)) < 0){
			fprintf(stderr, "Error accepting connection from a client");
			continue;
		}
		pthread_create(&tid, NULL, &client_response_worker, cli_sock);
	}
}