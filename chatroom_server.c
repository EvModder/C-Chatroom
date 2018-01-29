//Team: Nathaniel Leake 424003778, Chase Elander ###00####
//To compile: "make" or "gcc chatroom_server.c";
//Server available at chatroom-438.cf, or you can run your own

#include "interface.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <stdarg.h>

// cli_count = # currently connected, open_slot = 1st available
static unsigned int cli_count = 0, open_slot = 0;

typedef struct{
	struct sockaddr_in addr;// Client address
	int sockfd;// Client socket fd
	char name[MAX_DATA];// Client name
	int id;// Client index (unique identifier)
} client_t;

client_t* clients[MAX_CLIENTS];

int add_client(client_t*);
void del_client(int);
void broadcast(char*);
void send_message(char*, int);
void send_direct_message(char*, client_t*);

// Mutex for printf. Wish this wasn't required.
static pthread_mutex_t printf_mutex;
int sync_printf(const char *format, ...){
	va_list args;
	va_start(args, format);
	pthread_mutex_lock(&printf_mutex);
	vprintf(format, args);
	pthread_mutex_unlock(&printf_mutex);
	va_end(args);
}

//Nice utility function for converting addresses to strings
void addr_to_string(struct sockaddr_in addr, char* buffer){
	sprintf(buffer, "%d.%d.%d.%d",
		addr.sin_addr.s_addr & 0xFF,
		(addr.sin_addr.s_addr & 0xFF00)>>8,
		(addr.sin_addr.s_addr & 0xFF0000)>>16,
		(addr.sin_addr.s_addr & 0xFF000000)>>24);
}

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
	if(clients[i]){
		char buffer[MAX_DATA];
		snprintf(buffer, MAX_DATA, "%s has disconnected\n", clients[i]->name);
		broadcast(buffer);

		// Open this space for a new client
		free(clients[i]);
		clients[i] = NULL;
		if(i < open_slot) open_slot = i;
		--cli_count;
	}
}

// Send a message to a specific client
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
	if(LOG_MESSAGES) sync_printf(msg);
}

// Same as send_message except sent to everyone
void broadcast(char* msg){
	send_message(msg, -1);
	if(!LOG_MESSAGES) sync_printf(msg);
}

// This is the worker thread for a client (used to initialize pthreads)
void* client_listener_worker(client_t* client){
	// buffer to hold messages received from client
	char buffer[MAX_DATA];
	bzero(buffer, MAX_DATA);

	// A timeout for receiving the join message from a client
	struct timeval tv = {60, 0};// 1 minute timeout for first message
	setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	if(!USE_USERNAMES || read(client->sockfd, buffer, MAX_DATA) > 0){
		// This isn't required for this hw assignment, but usernames are nice
		if(USE_USERNAMES){
			buffer[MAX_DATA-2] = '\0';
			strcpy(client->name, buffer);
		}
		tv.tv_sec = 600;// 1 hour timeout if inactive
		setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

		// Read until there is an error, the connection dies, or the client says "QUIT"
		while(read(client->sockfd, buffer, MAX_DATA) > 0 && !startswith(buffer, "QUIT")){
			// We got a message from this client!
			// Send this message to the other clients
			if(USE_USERNAMES){
				strcat(client->name, ": ");
				send_message(client->name, client->id);
				client->name[strlen(client->name)-2] = '\0';
			}
			send_message(buffer, client->id);
		}
	}
	// Once we are no longer able to access this client, kick them
	del_client(client->id);

	// Detaching a thread is nice because it means the system can
	// throw it out immediately (we aren't going to use it again).
	// However, it was throwing strange errors...
	// pthread_detach(pthread_self());
	return NULL;
}

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr, "Usage: Enter a port number\n");
		fprintf(stderr, "Example: ./server.o 59255\n");
		return EXIT_FAILURE;
	}
	unsigned short port = atoi(argv[1]);

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
		perror("Error");
		exit(EXIT_FAILURE);
	}
	printf("Server started on port: %d\n", port);
	printf("Beginning loop to wait for clients...\n");

	pthread_t tid;
	pthread_mutex_init(&printf_mutex, NULL);
	while(1){
		if((cli_sock = accept(serv_sock, (struct sockaddr*) &cli_addr, &addr_sz)) < 0){
			fprintf(stderr, "Error in accepting connection from a client");
			continue;
		}
		// Create new client instance
		client_t* client = (client_t*) malloc(sizeof(client_t));
		client->addr = cli_addr;
		client->sockfd = cli_sock;
		addr_to_string(cli_addr, client->name);

		// If we can add another client, set up a new pthread to manage it
		if(add_client(client)){
			pthread_create(&tid, NULL, &client_listener_worker, client);
			sync_printf("New client joined! (%s)\n", client->name);
		}
		else{
			send_direct_message("Sorry, the chat server is full.", client);
			sync_printf("Unable to add new client (MAX_CLIENTS reached)\n");
			close(cli_sock);
			free(client);
		}

		// I've been told this will reduce CPU usage
		sleep(1);
	}
}