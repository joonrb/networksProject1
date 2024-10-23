#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>       // For getcwd() and chdir()
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
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    // Send and receive commands
    while (1) {
        printf("ftp> ");
        fflush(stdout);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            // EOF or error
            break;
        }
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

        /*the code logic is as follows:
        - if the command starts with '!', it is a local command and should be executed on the client side
        - otherwise, it is a server command and should be executed on the server side
        */
        //Check if the command starts with '!'
        if (buffer[0] == '!') {
            // Handle local commands
            if (strcmp(buffer, "!PWD") == 0) { //Code for !PWD command
                char cwd[BUFFER_SIZE];  //Declare a buffer to store the current working directory
                if (getcwd(cwd, sizeof(cwd)) != NULL) { //If successful, get the current working directory
                    printf("Local current directory: %s\n", cwd); //Print the current working directory
                } else {
                    perror("getcwd() error"); //If getting the directory failed, print an error message
                }
            } else if (strncmp(buffer, "!CWD ", 5) == 0) { //Code for !CWD command
                // Get the directory name
                char *dir = buffer + 5;
                if (chdir(dir) == 0) { //If changing the directory is successful, print a success message
                    printf("Local directory changed to %s\n", dir);
                } else {
                    perror("chdir() error"); //If changing the directory failed, print an error message
                }
            } else if (strcmp(buffer, "!LIST") == 0) {
                // Execute 'ls' command to list local directory
                int ret = system("ls"); //Execute the 'ls' command
                if (ret == -1) {
                    perror("system() error");
                }
            } else {
                printf("Unknown local command\n"); //If the command is not recognized, print an error message
            }
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            send(sock, buffer, strlen(buffer), 0);
            while ((bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) { //Receive the response from the server
                buffer[bytes_received] = '\0';
                printf("%s", buffer);
                if (strstr(buffer, "226") != NULL) { //If the response contains "226", break the loop
                    break;
                }
            }
        } else {
            // Send command to server
            if (send(sock, buffer, strlen(buffer), 0) < 0) {
                perror("send() failed");
                break;
            }

            bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0); //Receive the response from the server
            if (bytes_received > 0) { //If the response is received successfully, print it
                buffer[bytes_received] = '\0'; //Add a null terminator to the end of the response
                printf("Server: %s", buffer); //Print the response
            } else if (bytes_received == 0) { //If the connection is closed by the server, print a message and break the loop
                printf("Connection closed by server.\n");
                break;
            } else {
                perror("recv() failed"); //If there is an error in receiving the response, print an error message and break the loop
                break;
            }
        }
    }

    close(sock);
    return 0;
}
