#ifndef common
#define common

#define MAX_STRING_LENGTH 256

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Define error codes
enum RC
{
    SUCCESS = 0,
    ERROR = 1,
    USER_ERROR_2 = 2,
    OTHER_ERROR_3 = 3,
    OTHER_ERROR_4 = 4
};

#define MAX_IP_LENGTH 16 // Tamaño máximo de una dirección IP (incluyendo el carácter nulo)
#define MAX_PORT 65535   // Puerto máximo permitido
#define MAX_ARGS_LENGTH 2

typedef struct
{
    char user_name[MAX_STRING_LENGTH];
    char file_name[MAX_STRING_LENGTH];
    char description[MAX_STRING_LENGTH];
} Publication;

typedef struct
{
    char user_name[MAX_STRING_LENGTH];
    char ip[MAX_IP_LENGTH];
    int port;
} USER_CONNECTED;

typedef struct
{
    char key[MAX_STRING_LENGTH];
    char time[MAX_STRING_LENGTH];
    char user_name[MAX_STRING_LENGTH];
    char args[MAX_ARGS_LENGTH][MAX_STRING_LENGTH];
} client_operation;

// Creates a server socket bound to a specified address and port, using a given socket type.
int serverSocket(unsigned int addr, int port, int type);

// Accepts a connection on a server socket descriptor.
int serverAccept(int sd);

// Creates a client socket and connects it to a specified remote address and port.
int clientSocket(char *remote, int port);

// Closes a socket descriptor.
int closeSocket(int sd);

// Converts a double value from host byte order to network byte order.
double htond(double value);

// Converts a double value from network byte order to host byte order.
double ntohd(double value);

// Sends a message over a specified socket.
int sendMessage(int socket, char *buffer, int len);

// Receives a message from a specified socket.
int recvMessage(int socket, char *buffer, int len);

// Function to write a line to a file descriptor
ssize_t writeLine(int fd, char *buffer);

// Function to read a line from a file descriptor
ssize_t readLine(int fd, char *buffer, size_t n);

#endif
