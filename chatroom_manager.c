//Team: Nathaniel Leake 42400377, Chase Elander ###00####
//To compile: "make" or "gcc chatroom_server.c";
//Server available at chatroom-438.cf, or you can run your own

#include "interface.h"
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdarg.h>

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