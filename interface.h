/*****************************************************************
 * FILENAME :        interface.h
 *
 *    Functions and datastructures for the interface
 *    between users and the client program.
 *
 *    DO NOT MODIFY THIS FILE.  I CAN DO WHATEVER I WANT, OKAY!?
 *
 * Version: 1.0
 ******************************************************************/
#ifndef INTERFACE_H_
#define INTERFACE_H_
// #include <ctype.h>
#include <stdio.h>
#include <string.h>

// Maximum size of data for the communication using TCP/IP
#define MAX_DATA 256

// Maximum number of clients per Chat Server
#define MAX_CLIENTS 128

// Wether to require/enable usernames for clients
#define USE_USERNAMES 0

// Wether chatrooms should log all messages
#define LOG_MESSAGES 1

/*
 * This enum represents the result of a command.
 * Based on the response from the server,
 * you need to figure out the status.
 */
enum Status{
	SUCCESS,
	FAILURE_ALREADY_EXISTS,
	FAILURE_NOT_EXISTS,
	FAILURE_INVALID,
	FAILURE_UNKNOWN
};

/*
 * Reply structure is designed to be used for displaying the
 * result of the command that has been sent to the server.
 * For example, in the "process_command" function, you should
 * declare a variable of Reply structure and fill it based on
 * the type of command and the result.
 *
 * - CREATE and DELETE command:
 * Reply reply;
 * reply.status = one of values in Status enum;
 *
 * - JOIN command:
 * Reply reply;
 * reply.status = one of values in Status enum;
 * reply.num_members = # of members
 * reply.port = port number;
 *
 * - LIST command:
 * Reply reply;
 * reply.status = one of values in Status enum;
 * reply.list_room = list of rooms that have been create;
 *
 * This structure is not for communicating between server and client.
 * You need to design your own rules for the communication.
 */
struct Reply{
	enum Status status;

	union{
		// Below structure is only for the "JOIN <chatroom name>" command
		struct{
			// # of members that have been joined the chatroom
			int num_member;
			// Address of host (in case system is distributed)
			char host[MAX_DATA];
			// port number to join the chatroom
			int port;
		};

		// list_room is only for the "LIST" command
		// contains the list of rooms that have been created
		char list_room[MAX_DATA];
	};
};

/*
 * DO NOT MODIFY THIS FUNCTION
 * This function convert input string to uppercase.
 */
void touppercase(char* str, int n){
	int i;
	for(i=0; str[i]; ++i) str[i] = toupper((unsigned char)str[i]);
}

int startswith(char* str, char* prefix){
	int i;
	for(i=0; str[i] && prefix[i] == str[i]; ++i);
	return !prefix[i];
}

/*
 * DO NOT MODIFY THIS FUNCTION
 * This function displays a title, commands that user can use.
 */
void display_title(){
	printf("\n========= CHAT ROOM CLIENT =========\n");
	printf(" Command Lists and Format:\n");
	printf(" CREATE <name>\n");
	printf(" DELETE <name>\n");
	printf(" JOIN <name>\n");
	printf(" QUIT\n");
	printf(" LIST\n");
	printf("=====================================\n");
}

/*
 * DO NOT MODIFY THIS FUNCTION
 * This function prompts a user to enter a command.
 */
void get_command(char* comm, const int size){
	printf("Command> ");
	fgets(comm, size, stdin);
	comm[strlen(comm) - 1] = '\0';
}

/*
 * DO NOT MODIFY THIS FUNCTION
 * This function prompts a user to enter a command.
 */
void get_message(char* message, const int size){
	fgets(message, size, stdin);
	message[strlen(message) - 1] = '\0';
}


/*
 * DO NOT MODIFY THIS FUNCTION.
 * You should call this function to display the message from
 * other clients in a currently joined chatroom.
 */
void display_message(char* message){
	printf("> %s", message);
}

void display_reply(char* comm, const struct Reply reply){
	switch(reply.status) {
		case SUCCESS:
			printf("Command completed successfully\n");
			if(strncmp(comm, "JOIN", 4) == 0) {
				printf("#Members: %d\n", reply.num_member);
				printf("Host: %d, Port: %d\n", reply.host, reply.port);
			}
			else if(strncmp(comm, "LIST", 4) == 0) {
				printf("List: %s\n", reply.list_room);
			}
			break;
		case FAILURE_ALREADY_EXISTS:
			printf("Input chatroom name already exists, command failed\n");
			break;
		case FAILURE_NOT_EXISTS:
			printf("Input chatroom name does not exists, command failed\n");
			break;
		case FAILURE_INVALID:
			printf("Invalid command, command failed\n");
			break;
		case FAILURE_UNKNOWN:
			printf("Unknown error, command failed\n");
			break;
		default:
			printf("Invalid reply status, try updating your client\n");
			break;
	}
}

#endif // INTERFACE_H_
