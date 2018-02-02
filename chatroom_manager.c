//Team: Nathaniel Leake 424003778, Chase Elander ###00####
//To compile: "make" or "gcc chatroom_server.c";
//Server available at chatroom-438.cf, or you can run your own

#include "interface.h"
#include <netinet/in.h>
#include <stdlib.h>

int REPLY_SIZE = sizeof(struct Reply);
int SERVER_PORT = 59254;
int START_PORT = 4000;

void create_chatroom(int port, char * name){
	
	char buff[526];

	for (auto const& t : (* chatroom_map)){ //check that a chatroom with this
		if(strcmp(t.first.c_str(), name) == 0) { //  name does not already exists
			//it does already exist
			strcpy(buff, FAILURE_ALREADY_EXISTS.c_str());
			send(port, buff, 526, 0); //Send the failure
			return;
		}
	}

	//The chatroom doesn't exist yet

	//initialize new chatroom and push into chatroom_map
	int chat_socket_temp = passiveTCPConnection(chat_port_temp);
	struct Chatroom * chat = new Chatroom();
	chat->socket = chat_socket_temp;
	chat->port = chat_port_temp;
	chat->num_member = 0;
	chat->name = string(name);
	(*chatroom_map)[name] = chat;

	chat_port_temp++; //iterate chat_port_temp to ensure unique ports

	string reply_temp = SUCCESS + " 0 " + to_string(chat->port);
	strcpy(buff, reply_temp.c_str());
	send(sd, buff, 526, 0); //Send it back to client

/*                            Original Create chatroom code
	char str[12];
	sprintf(str, "%d", port);
	char *argv[]={"server.o", str, NULL};
	printf("Starting chatroom on port %d\n", port);
	if(fork() == 0) execv("./server.o", argv);
	*/
}

struct Chatroom {
	int socket;
	int port;
	int num_members;
	string name;
	pthread_t * thread;
	vector<int> clients;
};

map<string, struct Chatroom * > *chatroom_map;

int chat_port_temp = START_PORT;

void list_command(){
	string reply_temp = SUCCESS + " 0 0 ";
	for (auto const& t : (*chatroom_map)){
		reply_temp = reply_temp + t.first + ",";
	}

	//Send it back to Client
	return;
}
void join_command(char * name){
	string reply_temp = "";

	auto t = (chatroom_map).find(string(name));//verify it exists
	if( t == (*chatroom_map).end()){
		reply_temp = "FAILURE_NOT_EXISTS -1 -1";
	}
	else{
		struct Chatroom * ch = (*chatroom_map)[string(name)];
			ch ->num_member +=1;
			reply_temp = "SUCCESS " + to_string(c->num_member) + " " + to_string(c->port);
	}

	//Send back to client (maybe with write command?)
}

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
		list_command();
	}
	else if(startswith(command, "JOIN ")){
		// substr(5) to get chatroom name
		join_command(substr(5));
	}
	else if(startswith(command, "CREATE ")){
		//TODO: idk something for sure
		pthread_t tid;
		pthread_create(&tid, NULL, &create_chatroom, ++SERVER_PORT);
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
	unsigned short port = argc > 1 ? atoi(argv[1]) : SERVER_PORT;

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
