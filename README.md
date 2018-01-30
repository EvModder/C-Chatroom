438-Chatroom Server/Client
======
A simple chatroom program in C using Berkeley sockets.

### Compilation
A makefile is provided. simply run `make`\
For windows systems, use `gcc <file> -lpthread -o <file>.o`
#### Resulting binaries
* `manager.o`
* `server.o`
* `client.o`

## How-to-use
Firstly, run the Chatroom Manager (`manager.o`). This program listens for clients and handles commands sent over the network. Supported commands are `JOIN <chatroom name>`, `CREATE <chatroom name>`, `DELETE <chatroom name>`, and `LIST`.  If a client chooses the `QUIT` command, it is not sent to the Chatroom Manager but rather the client terminates immediately.  This can also be accomplished with `Ctrl-C` (or `Cmd-C` on OS X).\
\
To interact with the Chatroom Manager, run `client.o`.  This program will input commands from a user that are then sent across the network as previously described.  Once a `JOIN` command has completed, the user will enter 'Chat-mode', where all messages they enter will be sent to a chatroom host (an instance of `server.o`) and received by all other connected users.\
\
##### Live Demo
For demonstration purposes, a sample Manager server can be connected to at `chat-room-438.cf` on port `59254`
