#include "ftp_commands.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

void strip_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';  // Replace newline with null terminator
    }
}

void handle_cwd(int client_sock, char *dir) {
    char buffer[BUFFER_SIZE] = {0};  // Initialize buffer

    // Check if dir is NULL before using it
    if (dir == NULL) {
        snprintf(buffer, BUFFER_SIZE, "550 Invalid directory.\n");
        send(client_sock, buffer, strlen(buffer), 0);
        return;
    }

    // Strip leading spaces and trailing newline
    while (*dir == ' ') dir++;  // Skip leading spaces
    strip_newline(dir);  // Strip newline character at the end of the string

    if (chdir(dir) == 0) {
        char cwd[BUFFER_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            snprintf(buffer, BUFFER_SIZE, "200 directory changed to %s\n", cwd);
        } else {
            snprintf(buffer, BUFFER_SIZE, "550 Error getting current directory: %s\n", strerror(errno));
        }
    } else {
        snprintf(buffer, BUFFER_SIZE, "550 No such file or directory: %s\n", strerror(errno));
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

void handle_local_cwd(char *dir) {
    while(*dir == ' ') dir++; // skip spaces
    if (*dir == '\0') {
        printf("Usage: !CWD <directory>\n");
    } else if (chdir(dir) == 0) {
        printf("Local directory changed to %s\n", dir);
    } else {
        perror("chdir() error");
    }
}

void handle_local_pwd() {
    char cwd[BUFFER_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Local current directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
}