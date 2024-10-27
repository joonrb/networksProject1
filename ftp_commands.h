#ifndef FTP_COMMANDS_H
#define FTP_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void handle_cwd(int client_sock, char *dir);
void handle_pwd(int client_sock);
void handle_local_cwd(char *dir);
void handle_local_pwd();

#endif // FTP_COMMANDS_H