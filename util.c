#include "ftp_commands.h"
#include "util.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

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
    char cwd[BUFFER_SIZE];

    // Skip leading spaces
    while(*dir == ' ') dir++;

    if (*dir == '\0') {
        // No argument provided, change to home directory
        if (chdir(getenv("HOME")) == 0) {
            getcwd(cwd, sizeof(cwd));
            snprintf(buffer, BUFFER_SIZE, "250 Directory changed to %s\n", cwd);
        } else {
            snprintf(buffer, BUFFER_SIZE, "550 Failed to change to home directory.\n");
        }
    } else {
        // Argument provided
        if (chdir(dir) == 0) {
            getcwd(cwd, sizeof(cwd));
            snprintf(buffer, BUFFER_SIZE, "250 Directory changed to %s\n", cwd);
        } else {
            snprintf(buffer, BUFFER_SIZE, "550 Failed to change directory. %s\n", strerror(errno));
        }
    }
    send(client_sock, buffer, strlen(buffer), 0);
}

void handle_pwd(int client_sock, char *args) {
    char buffer[BUFFER_SIZE];
    char cwd[BUFFER_SIZE];

    // Skip leading spaces
    while(*args == ' ') args++;

    if (*args != '\0') {
        // Arguments provided, which is incorrect for PWD
        snprintf(buffer, BUFFER_SIZE, "501 Syntax error in parameters or arguments.\r\n");
    } else {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            snprintf(buffer, BUFFER_SIZE, "257 \"%s\" is the current directory\r\n", cwd);
        } else {
            snprintf(buffer, BUFFER_SIZE, "550 Error getting current directory: %s\r\n", strerror(errno));
        }
    }
    send(client_sock, buffer, strlen(buffer), 0);
}

int create_client_socket() {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    return sock;
}

void handle_local_commands(char *buffer) {
    if (strcmp(buffer, "!PWD") == 0) {
        char cwd[BUFFER_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Local current directory: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
    } else if (strncmp(buffer, "!CWD", 4) == 0) {
        char *dir = buffer + 4;
        while(*dir == ' ') dir++; // skip spaces
        if (*dir == '\0') {
            printf("Usage: !CWD <directory>\n");
        } else if (chdir(dir) == 0) {
            printf("Local directory changed to %s\n", dir);
        } else {
            perror("chdir() error");
        }
    } else if (strcmp(buffer, "!LIST") == 0) {
        int ret = system("ls");
        if (ret == -1) {
            perror("system() error");
        }
    } else {
        printf("Unknown local command\n");
    }
}

void handle_server_commands(int sock, char *buffer) {
    int bytes_received;
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("send() failed");
        return;
    }

    if (strncmp(buffer, "LIST", 4) == 0) {
        while ((bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
            if (strstr(buffer, "226") != NULL) {
                break;
            }
        }
    } else {
        bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Server: %s", buffer);
        } else if (bytes_received == 0) {
            printf("Connection closed by server.\n");
        } else {
            perror("recv() failed");
        }
    }
}

void handle_local_cwd(char *dir) {
    char cwd[BUFFER_SIZE];

    // Skip leading spaces
    while(*dir == ' ') dir++;

    if (*dir == '\0') {
        // No argument provided, change to home directory
        if (chdir(getenv("HOME")) == 0) {
            getcwd(cwd, sizeof(cwd));
            printf("Local directory changed to %s\n", cwd);
        } else {
            printf("Failed to change to home directory: %s\n", strerror(errno));
        }
    } else {
        // Argument provided
        if (chdir(dir) == 0) {
            getcwd(cwd, sizeof(cwd));
            printf("Local directory changed to %s\n", cwd);
        } else {
            printf("Failed to change directory: %s\n", strerror(errno));
        }
    }
}

void handle_local_pwd(char *args) {
    char cwd[BUFFER_SIZE];

    // Skip leading spaces
    while(*args == ' ') args++;

    if (*args != '\0') {
        // Arguments provided, which is incorrect for PWD
        printf("Error: PWD command does not accept any arguments.\n");
    } else {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Local current directory: %s\n", cwd);
        } else {
            printf("Error getting current directory: %s\n", strerror(errno));
        }
    }
}
