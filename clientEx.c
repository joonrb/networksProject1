#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 21
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
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

    // Receive and print the welcome message
    recv(sock, buffer, BUFFER_SIZE - 1, 0);
    printf("%s", buffer);

    // Send and receive commands
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

        send(sock, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "QUIT") == 0) {
            break;
        }

        recv(sock, buffer, BUFFER_SIZE - 1, 0);
        printf("Server: %s", buffer);
    }

    close(sock);
    return 0;
}
