//Team: Nathaniel Leake 42400377, Chase Elander ###00####
//To compile: "make" or "gcc chatroom_client.c";
//Server available at chatroom-438.cf, or you can run your own

#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "interface.h"

/*
 * TODO: IMPLEMENT BELOW THREE FUNCTIONS
 */
int connect_to(const char* host, const unsigned short port);
struct Reply process_command(const int sockfd, char* command);
void enter_chatmode(const char* host, const int port);

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

	display_title();
	while(1){
		// Read in a command from the user
		char command[MAX_DATA];
		get_command(command, MAX_DATA);
		touppercase(command, strlen(command)-1);

		// Exiting the program
		if(startswith(command, "EXIT")) break;

		// Otherwise, send this command to the Host Server
		int sockfd = connect_to(host_addr, port);
		if(sockfd < 0) return EXIT_FAILURE;

		struct Reply reply = process_command(sockfd, command);
		display_reply(command, reply);

		if(reply.status == SUCCESS && startswith(command, "JOIN")){
			enter_chatmode(reply.host, reply.port);
		}
	}
	return EXIT_SUCCESS;
}

/*
 * Connect to the server using given host and port information
 *
 * @parameter host    host address given by command line argument
 * @parameter port    port given by command line argument
 *
 * @return socket fildescriptor
 */
int connect_to(const char* host, const unsigned short port){
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error getting socket!");
		return sockfd;
	}
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(host);
	serv_addr.sin_port = htons(port);
	if(connect(sockfd, (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0){
		perror("Error connecting to server!");
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
	Reply reply;
	reply.status = FAILURE_UNKNOWN;

	// Write command to server
	if(write(sockfd, message, MAX_DATA) < 0){
		perror("Unable to send message to server");
		return reply:
	}
	// Receive response from server with 10 second timeout
	struct timeval tv = {10, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	char response[MAX_DATA];
	if(read(sockfd, response, MAX_DATA) <= 0){
		perror("No response from server (or invalid)");
		return reply:
	}

	// ------------------------------------------------------------
	// GUIDE 3:
	// Then, you should create a variable of Reply structure
	// provided by the interface and initialize it according to
	// the result.
	//
	// For example, if a given command is "JOIN room1"
	// and the server successfully created the chatroom,
	// the server will reply a message including information about
	// success/failure, the number of members and port number.
	// By using this information, you should set the Reply variable.
	// the variable will be set as following:
	//
	// Reply reply;
	// reply.status = SUCCESS;
	// reply.num_member = number;
	// reply.port = port;
	//
	// "number" and "port" variables are just an integer variable
	// and can be initialized using the message fomr the server.
	//
	// For another example, if a given command is "CREATE room1"
	// and the server failed to create the chatroom becuase it
	// already exists, the Reply varible will be set as following:
	//
	// Reply reply;
	// reply.status = FAILURE_ALREADY_EXISTS;
	//
	// For the "LIST" command,
	// You are suppose to copy the list of chatroom to the list_room
	// variable. Each room name should be seperated by comma ','.
	// For example, if given command is "LIST", the Reply variable
	// will be set as following.
	//
	// Reply reply;
	// reply.status = SUCCESS;
	// strcpy(reply.list_room, list);
	//
	// "list" is a string that contains a list of chat rooms such
	// as "r1,r2,r3,"
	// ------------------------------------------------------------
	char buffer[MAX_DATA];
	bzero(buffer, MAX_DATA);

	// REMOVE below code and write your own Reply.
	struct Reply reply;
	reply.status = SUCCESS;
	reply.num_member = 5;
	reply.port = 11024;
	return reply;
}

// This function is NOT MY WORK
// Credit: cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
// It returns true (1) if there is new input on stdin
int kbhit(){
    struct timeval tv = {1, 0};//1s
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
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
 *
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
		if(startswith(message, "QUIT")) break;
		display_message(message);
	}
	if(startswith(message, "QUIT")) printf("Kicked from server :O\n");
	else printf("Connection closed\n");

	// Close the pthread if it's still going
	pthread_cancel(tid);

	close(chat_server);
	printf("Exiting ChatMode\n");
	return;

	// ------------------------------------------------------------
	// GUIDE 2:
	// Once the client have been connected to the server, we need
	// to get a message from the user and send it to server.
	// At the same time, the client should wait for a message from
	// the server.
	// ------------------------------------------------------------

	// ------------------------------------------------------------
	// IMPORTANT NOTICE:
	// 1. To get a message from a user, you should use a function
	// "void get_message(char*, int);" in the interface.h file
	//
	// 2. To print the messages from other members, you should use
	// the function "void display_message(char*)" in the interface.h
	//
	// 3. Once a user entered to one of chatrooms, there is no way
	//    to command mode where the user  enter other commands
	//    such as CREATE,DELETE,LIST.
	//    Don't have to worry about this situation, and you can
	//    terminate the client program by pressing CTRL-C (SIGINT)
	// ------------------------------------------------------------
}
