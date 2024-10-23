#include "util.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void send_file_list(int client_sock) {
    DIR *d;
    struct dirent *dir;
    char list_buffer[BUFFER_SIZE];
    d = opendir(".");
    if (d) {
        send(client_sock, "150 Here comes the directory listing.\r\n", 39, 0);
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) { // Only list regular files
                snprintf(list_buffer, BUFFER_SIZE, "%s\r\n", dir->d_name);
                send(client_sock, list_buffer, strlen(list_buffer), 0);
            }
        }
        closedir(d);
        send(client_sock, "226 Directory send OK.\r\n", 24, 0);
    } else {
        send(client_sock, "550 Failed to open directory.\r\n", 31, 0);
    }
}

void handle_cwd(int client_sock, char *dir) {
    char buffer[BUFFER_SIZE];
    while(*dir == ' ') dir++; // skip spaces
    if(chdir(dir) == 0){
        char cwd[BUFFER_SIZE];
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            snprintf(buffer, BUFFER_SIZE, "200 directory changed to %s\n", cwd);
        } else {
            snprintf(buffer, BUFFER_SIZE, "550 Error getting current directory.\n");
        }
    } else {
        snprintf(buffer, BUFFER_SIZE, "550 No such file or directory.\n");
    }
    send(client_sock, buffer, strlen(buffer), 0);
}

void handle_pwd(int client_sock) {
    char buffer[BUFFER_SIZE];
    char cwd[BUFFER_SIZE];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        snprintf(buffer, BUFFER_SIZE, "257 \"%s\"\n", cwd);
    } else {
        snprintf(buffer, BUFFER_SIZE, "550 Error getting current directory.\n");
    }
    send(client_sock, buffer, strlen(buffer), 0);
}
