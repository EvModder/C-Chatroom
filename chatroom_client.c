//Team: Nathaniel Leake 424003778, Chase Elander 423005409
//To compile: "make" or "gcc chatroom_client.c";
//Server available at chatroom-438.cf, or you can run your own

#include "interface.h"
#include <netinet/in.h>
#include <stdlib.h>

int connect_to(const char* host, const unsigned short port);
struct Reply process_command(const int sockfd, char* command);
void enter_chatmode(const char* host, const int port);
int REPLY_SIZE = sizeof(struct Reply);

int main(int argc, char** argv){
	//If not specified, use defaults.
	char* host_addr = argc > 1 ? argv[1] : "chat-room-438.cf";
	unsigned short port = argc > 2 ? atoi(argv[2]) : 59254;
	printf("Host = %s, Port = %d\n", host_addr, port);
	// if(argc != 3){
	// 	fprintf(stderr, "Usage: Enter host address and port number\n");
	// 	fprintf(stderr, "Example: ./a.out chat-room-438.cf 59254\n");
	// 	return EXIT_FAILURE;
	// }

	// Ping the Chatroom Manager to make sure we can send commands
	int sockfd = connect_to(host_addr, port);
	if(sockfd < 0 || write(sockfd, "PING", 4) < 0){
		exit(EXIT_FAILURE);
	}

	display_title();
	char command[MAX_DATA];
	while(1){
		// Read a command from the user
		get_command(command, MAX_DATA);
		touppercase(command);

		// If exiting the program
		if(startswith(command, "EXIT")) break;

		// Connect to the Chatroom Manager
		if((sockfd = connect_to(host_addr, port)) < 0) exit(EXIT_FAILURE);

		// Send command to the Chatroom Manager
		struct Reply reply = process_command(sockfd, command);
		display_reply(command, reply);

		// If a successful JOIN, put this client in chatmode
		if(reply.status == SUCCESS && startswith(command, "JOIN")){
			enter_chatmode(reply.host, reply.port);
		}
	}
}

/*
 * Connect to the server using given host and port information
 * @parameter host    host address given by command line argument
 * @parameter port    port given by command line argument
 * @return socket fildescriptor
 */
int connect_to(const char* host, const unsigned short port){
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error getting socket");
		return sockfd;
	}
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(host);
	serv_addr.sin_port = htons(port);
	if(connect(sockfd, (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0){
		perror("Error connecting to server");
		return -1;
	}
	return sockfd;
}

/*
 * Send an input command to the server and return the result
 * @parameter sockfd   socket file descriptor to access the server
 * @parameter command  command that will be sent to the server
 * @return    Reply
 */
struct Reply process_command(const int sockfd, char* command){
	// If reply status is never set, assume FAILURE_UNKNOWN
	struct Reply reply;
	reply.status = FAILURE_UNKNOWN;

	// Write command to server
	if(write(sockfd, command, MAX_DATA) < 0){
		perror("Unable to send message to server");
		return reply;
	}
	// Receive reply from server with 10 second timeout
	struct timeval tv = {10, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	if(read(sockfd, &reply, REPLY_SIZE) <= 0){
		perror("Missing/Invalid response from server");
	}
	return reply;
}

void* input_listener_worker(int chat_server){
	// Separate thread for reading from IO and sending to server
	char message[MAX_DATA];
	do{
		get_message(message, MAX_DATA-1);
		if(startswith(message, "QUIT")) break;

		message[strlen(message)+1] = '\0';
		message[strlen(message)] = '\n';
	} while(write(chat_server, message, MAX_DATA) >= 0);

	if(!startswith(message, "QUIT")){
		// If we get here, there was an error in write()
		printf("Unable to send message to server\n");
	}
	close(chat_server);
	chat_server = 0;
	return NULL;
}

/*
 * Get into the chat mode
 * @parameter host     host address
 * @parameter port     port
 */
void enter_chatmode(const char* host, const int port){
	int chat_server = connect_to(host, port);
	if(chat_server < 0) return;

	printf("Entering ChatMode\n");

	char message[MAX_DATA];

	// Input a username and send it to the server
	if(USE_USERNAMES){
		printf("Please enter a username: \n");
		get_message(message, MAX_DATA);
		if(write(chat_server, message, MAX_DATA) < 0){
			perror("Unable to send message to server");
			return;
		}
	}

	pthread_t tid;
	pthread_create(&tid, NULL, &input_listener_worker, chat_server);

	// (Parent) Process to display messages from server
	while(read(chat_server, message, MAX_DATA) > 0 && chat_server){
		// if(startswith(message, "QUIT")) break;
		display_message(message);
	}
	// if(startswith(message, "QUIT")) printf("Kicked from server :O\n");
	// else printf("Connection closed\n");

	// Close the pthread if it's still going
	pthread_cancel(tid);

	close(chat_server);
	printf("Exiting ChatMode\n");
}