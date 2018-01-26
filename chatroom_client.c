//Team: Nathaniel Leake 42400377, Chase Elander ###00####
//To compile: "make" or "gcc chatroom_client.c";
//Server available at chatroom-438.cf, or you can run your own

//#include <netdb.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"

/*
 * TODO: IMPLEMENT BELOW THREE FUNCTIONS
 */
int connect_to(const char* host, const unsigned short port);
struct Reply process_command(const int sockfd, char* command);
void process_chatmode(const char* host, const int port);

int main(int argc, char** argv){
	//If not specified, use defaults.
	char* host_addr = argc > 1 ? argv[1] : "chat-room-438.cf";
	unsigned short port = argc > 2 ? atoi(argv[2]) : 59254;
	// if(argc != 3){
	// 	fprintf(stderr, "Usage: Enter host address and port number\n");
	// 	fprintf(stderr, "Example: ./a.out chat-room-438.cf 59254\n");
	// 	return EXIT_FAILURE;
	// }

	display_title();

	int sockfd = connect_to(host_addr, port);

	//TODO: This line is temporary to test chatmode! -
	// process_chatmode(host_addr, port);
	//------------------------------------------------


	while(1){
		char command[MAX_DATA];
		get_command(command, MAX_DATA);
		touppercase(command, strlen(command)-1);

		struct Reply reply = process_command(sockfd, command);
		display_reply(command, reply);

		if(reply.status == SUCCESS && startswith(command, "JOIN")){
			printf("Entering ChatMode\n");
			process_chatmode(reply.host, reply.port);
		}
	}
	close(sockfd);
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
	}
	return sockfd;
}

/*
 * Send an input command to the server and return the result
 *
 * @parameter sockfd   socket file descriptor to commnunicate
 *                     with the server
 * @parameter command  command will be sent to the server
 *
 * @return    Reply
 */
struct Reply process_command(const int sockfd, char* command){
	// ------------------------------------------------------------
	// GUIDE 1:
	// In this function, you are supposed to parse a given command
	// and create your own message in order to communicate with
	// the server. Surely, you can use the input command without
	// any changes if your server understand it. The given command
	// will be one of the followings:
	//
	// CREATE <name>
	// DELETE <name>
	// JOIN <name>
	// LIST
	//
	// -  "<name>" is a chatroom name that you want to create, delete,
	// or join.
	//
	// - CREATE/DELETE/JOIN and "<name>" are separated by one space.
	// ------------------------------------------------------------


	// ------------------------------------------------------------
	// GUIDE 2:
	// After you create the message, you need to send it to the
	// server and receive a result from the server.
	// ------------------------------------------------------------


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

/*
 * Get into the chat mode
 *
 * @parameter host     host address
 * @parameter port     port
 */
void process_chatmode(const char* host, const int port){
	// ------------------------------------------------------------
	// GUIDE 1:
	// In order to join the chatroom, you are supposed to connect
	// to the server using host and port.
	// You may re-use the function "connect_to".
	// ------------------------------------------------------------

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
