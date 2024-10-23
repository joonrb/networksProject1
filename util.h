#ifndef UTIL_H
#define UTIL_H

#include <dirent.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define PORT 21

void send_file_list(int client_sock);
void handle_cwd(int client_sock, char *dir);
void handle_pwd(int client_sock);
int create_client_socket();
void handle_local_commands(char *buffer);
void handle_server_commands(int sock, char *buffer);

#endif // UTIL_H
