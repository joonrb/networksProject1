#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h> 
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "client.h"

static int port_offset = 0;
char* client_dir = "./client";
ChildP children;

int main() {
    int server_fd, datasock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if (chdir(client_dir) != 0) {
		perror("chdir error");
	}

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure the client address
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr.sin_port = htons(0); 

    // Configure the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9002);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (bind(server_fd, (const struct sockaddr *)&client_addr, client_len) < 0) {
        perror("Socket bind failed");
        return 1;
    }

    // Connect to the server
    if (connect(server_fd, (struct sockaddr *)&server_addr, client_len) < 0) {
        perror("Connection failed");
        close(server_fd);
        return 1;
    }

    fd_set readfdset;
	FD_ZERO(&readfdset);

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

    printf("Connected to FTP server at %s\n", SERVER_IP);

    // Send and receive commands
    while (1) {
        FD_SET(server_fd, &readfdset);
        FD_SET(STDIN_FILENO, &readfdset);

        if (select(server_fd+1,&readfdset,NULL,NULL,NULL)<0) {
			perror("select");
			exit (-1);
		}
        if(FD_ISSET(server_fd, &readfdset)){
            handleMessage(server_fd);
        }
        
        if(FD_ISSET(STDIN_FILENO, &readfdset)){
            handleCommand(server_fd);
        }
    }

    close(server_fd);
    return 0;
}

void handleCommand(int server_fd){
    char buffer[BUFFER_SIZE];
    bzero(buffer,sizeof(buffer));

    if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

        // Check if the command is STOR
        if(strncmp(buffer, "STOR", 4) == 0){
            // Send PORT command first
            int data_listen_fd = portCom(server_fd);
            if(data_listen_fd < 0){
                fprintf(stderr, "Failed to send PORT command.\n");
                return;
            }

            // Wait for server's response to PORT command
            char response[BUFFER_SIZE];
            int bytes_received = recv(server_fd, response, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                perror("Failed to receive server response to PORT command");
                close(server_fd);
                exit(EXIT_FAILURE);
            }
            response[bytes_received] = '\0';
            printf("%s", response);

            // Check if the server accepted the PORT command
            if (strncmp(response, "200", 3) != 0) {
                fprintf(stderr, "Server did not accept PORT command.\n");
                return;
            }

            // Now handle STOR command
            storCom(server_fd, buffer, data_listen_fd);
        }
        else if(strncmp(buffer, "RETR", 4) == 0){
            retrCom(server_fd, buffer);
        }
        else if(strncmp(buffer, "LIST", 4) == 0){
            listCom(server_fd, buffer);
        }
        else {
            // For other commands, send them directly
            if (send(server_fd, buffer, strlen(buffer), 0) < 0) {
                perror("send failed");
            }
        }
    }
}


void handleMessage(int server_fd){
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(server_fd, buffer, BUFFER_SIZE, 0);

    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Server closed the connection.\n");
        } else {
            perror("recv failed");
        }
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';
    printf("%s", buffer);
    bzero(buffer,sizeof(buffer));
}

int portCom(int server_fd){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock, p1, p2;

    // Create a new socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Allow address reuse
    int value  = 1;
    setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    // Initialize client_addr
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;

    // Use INADDR_ANY or specific interface
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with actual IP if needed

    // Bind to port 0 to let the OS choose an available port
    client_addr.sin_port = htons(0);

    if (bind(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0) {
        perror("bind error");
        close(client_sock);
        return -1;
    }

    // Retrieve the assigned port number
    if (getsockname(client_sock, (struct sockaddr*) &client_addr, &client_len) < 0) {
        perror("getsockname error");
        close(client_sock);
        return -1;
    }

    // Extract IP and port information
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    unsigned int ip_parts[4];
    sscanf(ip_str, "%u.%u.%u.%u", &ip_parts[0], &ip_parts[1], &ip_parts[2], &ip_parts[3]);

    int port = ntohs(client_addr.sin_port);
    p1 = port / 256;
    p2 = port % 256;

    // Prepare the PORT command
    char msg[256];
    sprintf(msg, "PORT %u,%u,%u,%u,%d,%d", ip_parts[0], ip_parts[1], ip_parts[2], ip_parts[3], p1, p2);
    printf("Sending PORT command: %s\n", msg);

    // Start listening for the server's data connection
    if (listen(client_sock, 1) < 0) {
        perror("listen error");
        close(client_sock);
        return -1;
    }

    // Send the PORT command to the server
    send_msg(server_fd, msg);

    return client_sock;
}

void storCom(int server_fd, char* buffer, int data_listen_fd){
    int pid = fork();
    if(pid < 0){
        perror("fork error");
        close(server_fd);
        exit(1);
    }

    if(pid == 0){
        signal(SIGTERM, closeChild);
        children.command_fd = server_fd;
        children.data_listen_fd = data_listen_fd;
        children.file = fopen(buffer+5, "rb");
        if (!children.file) {
            perror("Failed to open file");
            closeChild(SIGTERM);
        }

        // Send STOR command
        send_msg(children.command_fd, buffer);

        // Wait for server's response to STOR command
        char response[BUFFER_SIZE];
        int bytes_received = recv(children.command_fd, response, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive server response to STOR command");
            closeChild(SIGTERM);
        }
        response[bytes_received] = '\0';
        printf("%s", response);

        // Check if the server is ready to receive the file
        if (strncmp(response, "150", 3) != 0 && strncmp(response, "125", 3) != 0) {
            fprintf(stderr, "Server did not accept STOR command.\n");
            closeChild(SIGTERM);
        }

        // Accept data connection
        int client_sock = accept(children.data_listen_fd, NULL, NULL);
        if(client_sock < 0){
            perror("Data connection error");
            closeChild(SIGTERM);
        }
        children.data_fd = client_sock;

        // Begin file transfer
        char sendFile[BUFFER_SIZE];
        int bytes_to_send;
        while((bytes_to_send = fread(sendFile, 1, BUFFER_SIZE, children.file)) > 0){
            if(send(children.data_fd, sendFile, bytes_to_send, 0) < 0){
                perror("Send error");
                closeChild(SIGTERM);
            }
        }

        // Close data connection and wait for server's transfer completion response
        close(children.data_fd);
        bytes_received = recv(children.command_fd, response, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("%s", response);
        }

        closeChild(SIGTERM);
    }
}

void retrCom(int server_fd, char* buffer){
    // Send PORT command first
    int data_listen_fd = portCom(server_fd);
    if(data_listen_fd < 0){
        fprintf(stderr, "Failed to send PORT command.\n");
        return;
    }

    // Wait for server's response to PORT command
    char response[BUFFER_SIZE];
    int bytes_received = recv(server_fd, response, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive server response to PORT command");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    response[bytes_received] = '\0';
    printf("%s", response);

    // Check if the server accepted the PORT command
    if (strncmp(response, "200", 3) != 0) {
        fprintf(stderr, "Server did not accept PORT command.\n");
        close(data_listen_fd);
        return;
    }

    // Send RETR command
    send_msg(server_fd, buffer);

    // Wait for server's response to RETR command
    bytes_received = recv(server_fd, response, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive server response to RETR command");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    response[bytes_received] = '\0';
    printf("%s", response);

    // Check if the server is ready to send the file
    if (strncmp(response, "150", 3) != 0 && strncmp(response, "125", 3) != 0) {
        fprintf(stderr, "Server did not accept RETR command.\n");
        close(data_listen_fd);
        return;
    }

    // Now fork a child process to handle data transfer
    int pid = fork();
    if(pid < 0){
        perror("fork error");
        close(server_fd);
        exit(1);
    }

    if(pid == 0){
        // Child process
        signal(SIGTERM, closeChild);
        children.data_listen_fd = data_listen_fd;

        // Accept data connection
        children.data_fd = accept(children.data_listen_fd, NULL, NULL);
        if(children.data_fd < 0){
            perror("Data connection error");
            closeChild(SIGTERM);
        }

        // Open file for writing
        char* fileName = buffer + 5; // Skip 'RETR ' (5 characters)
        children.file = fopen(fileName, "wb");
        if (!children.file) {
            perror("Failed to open file");
            closeChild(SIGTERM);
        }

        // Receive data from server and write to file
        char file_buffer[BUFFER_SIZE];
        int bytes_received_data;
        while ((bytes_received_data = recv(children.data_fd, file_buffer, BUFFER_SIZE, 0)) > 0) {
            if(fwrite(file_buffer, 1, bytes_received_data, children.file) < bytes_received_data){
                perror("File write error");
                closeChild(SIGTERM);
            }
        }

        fclose(children.file);
        close(children.data_fd);
        close(children.data_listen_fd);

        closeChild(SIGTERM);
    } else {
        // Parent process
        close(data_listen_fd); // Close listening socket in parent

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Wait for server's final response (e.g., 226 Transfer complete)
        bytes_received = recv(server_fd, response, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("%s", response);
        }
    }
}

void listCom(int server_fd, char* buffer){
    // Send PORT command first
    int data_listen_fd = portCom(server_fd);
    if(data_listen_fd < 0){
        fprintf(stderr, "Failed to send PORT command.\n");
        return;
    }

    // Wait for server's response to PORT command
    char response[BUFFER_SIZE];
    int bytes_received = recv(server_fd, response, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive server response to PORT command");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    response[bytes_received] = '\0';
    printf("%s", response);

    // Check if the server accepted the PORT command
    if (strncmp(response, "200", 3) != 0) {
        fprintf(stderr, "Server did not accept PORT command.\n");
        close(data_listen_fd);
        return;
    }

    // Send LIST command
    send_msg(server_fd, "LIST");

    // Wait for server's response to LIST command
    bytes_received = recv(server_fd, response, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive server response to LIST command");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    response[bytes_received] = '\0';
    printf("%s", response);

    // Check if the server is ready to send the directory listing
    if (strncmp(response, "150", 3) != 0 && strncmp(response, "125", 3) != 0) {
        fprintf(stderr, "Server did not accept LIST command.\n");
        close(data_listen_fd);
        return;
    }

    // Now fork a child process to handle data transfer
    int pid = fork();
    if(pid < 0){
        perror("fork error");
        close(server_fd);
        exit(1);
    }

    if(pid == 0){
        // Child process
        signal(SIGTERM, closeChild);
        children.data_listen_fd = data_listen_fd;

        // Accept data connection
        children.data_fd = accept(children.data_listen_fd, NULL, NULL);
        if(children.data_fd < 0){
            perror("Data connection error");
            closeChild(SIGTERM);
        }

        // Receive directory listing from server and display it
        char data_buffer[BUFFER_SIZE];
        int bytes_received_data;
        while ((bytes_received_data = recv(children.data_fd, data_buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(data_buffer, 1, bytes_received_data, stdout);
        }

        close(children.data_fd);
        close(children.data_listen_fd);

        closeChild(SIGTERM);
    } else {
        // Parent process
        close(data_listen_fd); // Close listening socket in parent

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Wait for server's final response (e.g., 226 Transfer complete)
        bytes_received = recv(server_fd, response, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("%s", response);
        }
    }
}

void closeChild(int sig) {
	// clean up and terminate the child process
	if (sig == SIGTERM) {
		/*if (children.command_fd > 2) {
			close(children.command_fd);
			children.command_fd = -1;
		}*/
		if (children.data_fd > 2) {
			close(children.data_fd);
			children.data_fd = -1;
		}
        if (children.data_listen_fd > 2) {
            close(children.data_listen_fd);
            children.data_listen_fd = -1;
        }
		if (!children.file) {
			fclose(children.file);
			children.file = NULL;
		}
		exit(0);
	}
}

void send_msg(int fd, char* msg) {
	send(fd, msg, strlen(msg)+1, 0);
}