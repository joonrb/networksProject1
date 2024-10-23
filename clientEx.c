#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>       // For getcwd() and chdir()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "util.h"

#define SERVER_IP "127.0.0.1"
#define PORT 21
#define BUFFER_SIZE 1024

int main() {
    int sock;
    char buffer[BUFFER_SIZE];

    sock = create_client_socket();
    if (sock < 0) {
        return 1;
    }

    printf("Hello!! Please Authenticate to run server commands\n");
    printf("1. type \"USER\" followed by a space and your username\n");
    printf("2. type \"PASS\" followed by a space and your password\n");
    printf("\"QUIT\" to close connection at any moment\n");
    printf("Once Authenticated, this is the list of commands:\n");
    printf("\"STOR\" + space + filename | to send a file to the server\n");
    printf("\"RETR\" + space + filename | to download a file from the server\n");
    printf("\"LIST\" | to list all the files under the current server directory\n");
    printf("\"CWD\" + space + directory | to change the current server directory\n");
    printf("\"PWD\" | to display the current server directory\n");
    printf("Add \"!\" before the last three commands to apply them locally\n");

    printf("Connected to FTP server at %s:%d\n", SERVER_IP, PORT);

    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    while (1) {
        printf("ftp> ");
        fflush(stdout);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "QUIT") == 0) {
            send(sock, buffer, strlen(buffer), 0);
            break;
        }

        if (buffer[0] == '!') {
            handle_local_commands(buffer);
        } else {
            handle_server_commands(sock, buffer);
        }
    }

    close(sock);
    return 0;
}
