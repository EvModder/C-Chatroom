//Team: Nathaniel Leake 424003778, Chase Elander ###00####
//To compile: "make" or "gcc chatroom_server.c";
//Server available at chatroom-438.cf, or you can run your own

#include "interface.h"
#include <netinet/in.h>
#include <stdlib.h>

int REPLY_SIZE = sizeof(struct Reply);

/*
 * Handle a command from a client and store the result in the
 * provided Reply struct
 * @parameter command  command from client to be handled
 * @parameter reply    struct to store the result
 */
void run_command(char* command, struct Reply* reply){
	// TODO: comment out debug lines for final submission
	printf("Received: %s\n", command);

	// TODO: Implement these
	if(startswith(command, "LIST")){

	}
	else if(startswith(command, "JOIN ")){
		// substr(5) to get chatroom name
	}
	else if(startswith(command, "CREATE ")){
		
	}
	else if(startswith(command, "DELETE ")){
		
	}
	reply->status = SUCCESS;
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
	if((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) <= 0 ||
		bind(serv_sock, (struct sockaddr*) &serv_addr, addr_sz) != 0 || 
		listen(serv_sock, 5))
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
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