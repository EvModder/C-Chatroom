// #include <netdb.h>
// #include <sys/types.h>
// #include <ctype.h>
// #include "interface.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#define MAX_DATA 256
#define MAX_CLIENTS	128

static unsigned int cli_count = 0, open_slot = 0;
void broadcast(char*);

typedef struct{
	struct sockaddr_in addr;// Client address
	int sockfd;// Client socket fd
	char name[128];// Client name
	int id;// Client index (unique identifier)
} client_t;

client_t* clients[MAX_CLIENTS];

// Called when a new client connects
// Returns false if there isn't enough space for the client
int add_client(client_t* client){
	if(++cli_count == MAX_CLIENTS){
		--cli_count;
		return 0;
	}
	int i;
	for(i=open_slot; i<MAX_CLIENTS; ++i){
		if(!clients[i]){
			client->id = i;
			clients[i] = client;
			open_slot = i+1;
			return 1;
		}
	}
	--cli_count;
	return 0;
}

// Called when a client disconnects or times out
void del_client(int i){
	char buffer[MAX_DATA];
	snprintf(buffer, MAX_DATA, "%s has disconnected from the server", clients[i]->name);
	broadcast(buffer);

	// Open this space for a new client
	clients[i] = NULL;
	if(i < open_slot) open_slot = i;
	--cli_count;
}

void send_direct_message(char* msg, client_t* client){
	// Write message
	if(client && write(client->sockfd, msg, strlen(msg)) < 0){
		// We seem unable to reach this client -- disconnect them.
		del_client(client->id);
	}
}

// Send a message to all clients (except the sender)
void send_message(char* msg, int id_from){
	int i, msg_sz = strlen(msg);
	for(i=0; i<MAX_CLIENTS; ++i){
		if(clients[i] && clients[i]->id != id_from){
			if(write(clients[i]->sockfd, msg, msg_sz) < 0){
				// We seem unable to reach this client -- disconnect them.
				del_client(i);
			}
		}
	}
}

// Same as send_message except sent to everyone
void broadcast(char* msg){
	send_message(msg, -1);
}

void* client_listener(client_t* client){
	char buffer[MAX_DATA];
	bzero(buffer, MAX_DATA);
	while(read(client->sockfd, buffer, MAX_DATA) > 0){
		//we got a message from this client!
		//TODO: if this message is a command, parse it
		//else: (send this chat to other clients)
		send_message(buffer, client->id);
	}
	del_client(client->id);
	free(client);
	pthread_detach(pthread_self());
	return NULL;
}

int main(int argc, char** argv){
	//If not specified, use this port as default.
	unsigned short port = argc > 1 ? atoi(argv[1]) : 59254;

	// Structures used for setting up the server
	int serv_sock, cli_sock, addr_sz;
	struct sockaddr_in serv_addr, cli_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	addr_sz = sizeof(serv_addr);

	// Open socket, bind it to a port, and listen for connections
	// If any of these fails, we terminate with an error message.
	if((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
		bind(serv_sock, (struct sockaddr*) &serv_addr, addr_sz) || 
		listen(serv_sock, 5))
	{
		// This is the most common/likely error by far
		if(errno == EADDRINUSE) error("Error: Address already in use!");

		//Everything else
		else perror("Error: ");
		return EXIT_FAILURE;
	}
	printf("Server started on port: %d\n", port);
	printf("Beginning loop to wait for clients...\n");

	pthread_t tid;
	while(1){
		if((cli_sock = accept(serv_sock, (struct sockaddr*) &cli_addr, &addr_sz)) < 0){
			fprintf(stderr, "Error in accepting connection from a client");
			continue;
		}
		// Create new client instance
		client_t* client = (client_t*) malloc(sizeof(client_t));
		client->addr = cli_addr;
		client->sockfd = cli_sock;
		// client->name = "Unnamed User";

		// If we can add another client, set up a new pthread to manage it
		if(add_client(client)){
			pthread_create(&tid, NULL, &client_listener, client);
			printf("New client joined!\n");
		}
		else{
			send_direct_message("Sorry, the chat server is full.", client);
			printf("Unable to add new client (MAX_CLIENTS reached)\n");
			close(cli_sock);
			free(client);
		}

		// I've been told this will reduce CPU usage
		sleep(1);
	}
}