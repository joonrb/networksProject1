#ifndef FTP_COMMANDS_H
#define FTP_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define BUFFER_SIZE 1024
// #define SERVER_IP "127.0.0.1"
// #define PORT 2121
// #define MAX_CONNECT 30

// Server-side functions
void handle_cwd(int client_sock, char *dir);
void handle_pwd(int client_sock);
void send_file_list(int client_sock);

// Client-side functions
void handle_local_cwd(char *dir);
void handle_local_pwd(const char *args);  // Made const since we don't modify it
void handle_local_list(const char *args); // Made const since we don't modify it

// Utility function declaration
//void send_msg(int fd, char *msg);

#endif // FTP_COMMANDS_H
