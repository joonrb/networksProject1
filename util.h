#ifndef UTIL_H
#define UTIL_H

#include <dirent.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void send_file_list(int client_sock);
void handle_cwd(int client_sock, char *dir);
void handle_pwd(int client_sock);

#endif // UTIL_H
