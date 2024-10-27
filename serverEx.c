#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h> 
#include <sys/select.h>
#include <errno.h>

#include "server.h"
#include "ftp_commands.h"

Login db[NUM_OF_USERS];
ChildP children;
char* server_dir = "./server";

int main(){
    int server_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    socklen_t server_len = sizeof(server_addr);

    User userList[MAX_CONNECT];
    for(int i = 0 ; i < MAX_CONNECT; i++){
        userList[i].userfd = -1;
        userList[i].auth = 0;
        userList[i].username = NULL;
        bzero(userList[i].dir, sizeof(userList[i].dir));
        sprintf(userList[i].dir, "/");
    }

    loadUser(db);
    if(chdir(server_dir) != 0){
        perror("chdir error");
        exit(1);
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        perror("Socket creation failed");
        return 1;
    }

    int value  = 1;
	setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));

    // Set up the server address
    memset(&server_addr, 0, server_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if(bind(server_fd, (struct sockaddr *)&server_addr, server_len) < 0) {
        perror("Socket bind failed");
        return 1;
    }

    // Listen for clients
    if(listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    fd_set all_sockets;
	fd_set ready_sockets;

    int max_socket_so_far = server_fd;

	FD_ZERO(&all_sockets);
	FD_SET(server_fd,&all_sockets);

    printf("FTP Server is listening on port %d\n", PORT);

    // Accept clients
    while(1){
        ready_sockets = all_sockets;

        if(select(max_socket_so_far+1,&ready_sockets,NULL,NULL,NULL)<0)
		{
			perror("select error");
			exit(EXIT_FAILURE);
		}

        for(int fd = 3 ; fd <= max_socket_so_far; fd++){
            if(FD_ISSET(fd, &ready_sockets)){
                if(fd == server_fd){
                    client_sock = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (client_sock < 0) {
                        perror("accept");
                    }

                    send_msg(client_sock, "220 Service ready for new user! \n");
                    printf("Connection established with user %d\n", fd);
                    printf("Their Port: %d\n", ntohs(client_addr.sin_port));

                    FD_SET(client_sock, &all_sockets);
                    if (client_sock > max_socket_so_far) {
                        max_socket_so_far = client_sock;
                    }

                    for(int i = 0; i < MAX_CONNECT; i++){
                        if(userList[i].userfd < 0){
                            userList[i].userfd = client_sock;
                            break;
                        }
                    }
                }
                else{
                    handleCommand(fd, &all_sockets, &max_socket_so_far, userList);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}


void loadUser(Login* db){
    FILE *file = fopen("users.csv", "r");
    if(!file){
        perror("Failed to open file");
        exit(1);
    }
    while(fscanf(file, "%49[^,],%49s\n", db[userNum].username, db[userNum].password) == 2){
        userNum++;
    }
    fclose(file);
}

void checkUser(User* userList, int index, int fd, char *buffer){
    if(strncmp(buffer + 4, " ", 1) && strncmp(buffer + 4, "\n", 1)){
        send_msg(fd, "202 command not implemented. \n");
    }
    else if(userList[index].username != NULL){
        send_msg(fd, "503 Bad sequence of commands. \n");
    }
    else{
        for(int i = 0; i < userNum; i++){
            if(strncmp(buffer + 5, db[i].username, sizeof(db[i].username)) == 0){
                userList[index].username = db[i].username;
                send_msg(fd, "331 Username OK, need password. \n");
                printf("Successful username verification. \n");
            }
        }
    }
}

void checkPass(User* userList, int index, int fd, char *buffer){
    if(strncmp(buffer + 4, " ", 1) && strncmp(buffer + 4, "\n", 1)){
        send_msg(fd, "202 command not implemented. \n");
    }
    else if(userList[index].username == NULL || userList[index].auth){
        send_msg(fd, "503 Bad sequence of commands. \n");
    }
    else{
        for(int i = 0; i < userNum; i++){
            if(strcmp(userList[index].username, db[i].username) == 0 && strncmp(buffer + 5, db[i].password, sizeof(db[i].password)) == 0){
                userList[index].auth = 1;
                send_msg(fd, "230 User logged in, proceed. \n");
                printf("Successful login. \n");
            }
        }
    }
}

void handleCommand(int fd, fd_set* allsocket, int* max_socket_so_far, User* userList){
    int index = -1;
    for(int i = 0; i < MAX_CONNECT; i++){
        if(userList[i].userfd == fd){
            index = i;
            break;
        }
    }
    if(index == -1){
        perror("you shouldn't see this");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    bzero(buffer,sizeof(buffer));
    int bytes_read;

    if((bytes_read = recv(fd, buffer, BUFFER_SIZE - 1, 0)) < 0) {
        perror("Error reading incoming stream\n");
        close(fd);
        userList[index].userfd = -1;
    }
    else if (bytes_read == 0) {
        close(fd);
        FD_CLR(fd, allsocket);
        userList[index].userfd = -1;
        userList[index].auth = 0;
        userList[index].username = NULL;
        bzero(userList[index].dir, sizeof(userList[index].dir));
        sprintf(userList[index].dir, "/");

        printf("Client %d disconnected\n", fd);
    }
    else{
        if (userList[index].buffer_len + bytes_read >= BUFFER_SIZE) {
            // Buffer overflow, handle error
            send_msg(fd, "500 Command too long.\n");
            userList[index].buffer_len = 0;
            return;
        }

        buffer[bytes_read] = '\0';
        if (strncmp("USER", buffer, 4) == 0) {
            checkUser(userList, index, fd, buffer);
        } 
        else if (strncmp("PASS", buffer, 4) == 0) {
            checkPass(userList, index, fd, buffer);
        }
        else if (strncmp("PORT", buffer, 4) == 0) {
            portCom(userList, index, fd, buffer);
        }
        else if(strncmp("STOR", buffer, 4) == 0){
            storCom(userList, index, fd, buffer);
        }
        else if(strncmp("RETR", buffer, 4) == 0){
            retrCom(userList, index, fd, buffer);
        }
        else if(strncmp("LIST", buffer, 4) == 0){
            listCom(userList, index, fd, buffer);
        }
        else if(strncmp("CWD", buffer, 3) == 0){
            if(!userList[index].auth){
                send_msg(fd, "530 Not logged in.\n");
                return;
            }
            handle_cwd(fd, buffer + 4);
        }
        else if(strcmp("PWD", buffer) == 0){
            if(!userList[index].auth){
                send_msg(fd, "530 Not logged in.\n");
                return;
            }
            handle_pwd(fd);
        }
        else if(strncmp("QUIT", buffer, 4) == 0){
            //send(client_sock, "221 Goodbye\n", 12, 0);
        }
        else {
            // Wrong commands 
            send_msg(fd, "202 Command not implemented.\n");
        }
    }
}

void portCom(User* userList, int index, int fd, char *buffer){
    // Trim trailing whitespace and control characters
    char *newline = strpbrk(buffer, "\r\n");
    if (newline) *newline = '\0';

    if (buffer[4] != ' ') {
        send_msg(fd, "202 command not implemented. PC\n");
        return;
    } else if (!userList[index].auth) {
        send_msg(fd, "530 Not logged in.\n");
        return;
    } else {
        int h1, h2, h3, h4, p1, p2;
        if (sscanf(buffer + 5, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
            send_msg(fd, "501 Syntax error in parameters or arguments.\n");
            return;
        }

        // Validate IP and port components
        if ((h1 | h2 | h3 | h4 | p1 | p2) & ~0xFF) {
            send_msg(fd, "501 Invalid IP address or port.\n");
            return;
        }

        char addr_str[INET_ADDRSTRLEN];
        snprintf(addr_str, sizeof(addr_str), "%d.%d.%d.%d", h1, h2, h3, h4);

        userList[index].port = (p1 << 8) | p2;

        if (inet_pton(AF_INET, addr_str, &(userList[index].addr)) != 1) {
            send_msg(fd, "501 Invalid IP address.\n");
            return;
        }

        printf("Ports Received: %d, %d, %d, %d, %d, %d \n", h1, h2, h3, h4, p1, p2);

        send_msg(fd, "200 PORT command successful.\n");
    }
}

void storCom(User* userList, int index, int fd, char *buffer){
    if(strncmp(buffer + 4, " ", 1) != 0 || strlen(buffer + 5) == 0){
        send_msg(fd, "501 Syntax error in parameters or arguments.\n");
        return;
    } else if(!userList[index].auth){
        send_msg(fd, "530 Not logged in.\n");
        return;
    } else if(userList[index].addr.s_addr == 0 || userList[index].port == 0) {
        send_msg(fd, "425 Use PORT or PASV first.\n");
        return;
    } else {
        // Send preliminary reply
        send_msg(fd, "150 Opening data connection.\n");

        int pid = fork();
        if(pid < 0){
            perror("fork error");
            exit(1);
        }
        if(pid == 0){
            // Child process
            // Handle data connection
            signal(SIGTERM, closeChild);

            // Prepare file paths
            char* fileName = buffer + 5; // Skip 'STOR ' (5 characters)
            char file[FILENAME_MAX];
            char temp_file[FILENAME_MAX];
            snprintf(file, FILENAME_MAX, "./%s%s/%s", userList[index].username, userList[index].dir, fileName);
            snprintf(temp_file, FILENAME_MAX, "%s.incomplete", file);

            // Open the temporary file for writing
            children.file = fopen(temp_file, "wb");
            if (!children.file) {
                perror("Failed to open file");
                // Can't send message to client here
                closeChild(SIGTERM);
            }

            printf("File okay, beginning data conenctions \n");

            // Open data connection
            if((children.data_fd = open_data_connection(userList[index].addr, userList[index].port)) < 0){
                // Can't send message to client here
                closeChild(SIGTERM);
            }

            // Receive data and write to file
            char file_buffer[BUFFER_SIZE];
            int bytes_read;
            while ((bytes_read = recv(children.data_fd, file_buffer, BUFFER_SIZE, 0)) > 0) {
                if(fwrite(file_buffer, 1, bytes_read, children.file) < bytes_read){
                    perror("File write error");
                    // Can't send message to client here
                    closeChild(SIGTERM);
                }
            }

            fclose(children.file);
            close(children.data_fd);

            // Rename the temporary file to the final file
            if(rename(temp_file, file) < 0){
                perror("Failed to rename file");
                // Can't send message to client here
                closeChild(SIGTERM);
            }

            closeChild(SIGTERM);
        } else {
            // Parent process
            // Wait for child process to complete
            int status;
            waitpid(pid, &status, 0);

            // Send transfer completion reply
            send_msg(fd, "226 Transfer complete.\n");
        }
    }
}


void retrCom(User* userList, int index, int fd, char *buffer){
    if(strncmp(buffer + 4, " ", 1) != 0 || strlen(buffer + 5) == 0){
        send_msg(fd, "501 Syntax error in parameters or arguments.\n");
        return;
    } else if(!userList[index].auth){
        send_msg(fd, "530 Not logged in.\n");
        return;
    } else if(userList[index].addr.s_addr == 0 || userList[index].port == 0) {
        send_msg(fd, "425 Use PORT or PASV first.\n");
        return;
    } else {
        // Send preliminary reply
        send_msg(fd, "150 Opening data connection.\n");

        int pid = fork();
        if(pid < 0){
            perror("fork error");
            exit(1);
        }
        if(pid == 0){
            // Child process
            signal(SIGTERM, closeChild);
            children.command_fd = fd;

            // Prepare file path
            char* fileName = buffer + 5; // Skip 'RETR ' (5 characters)
            char file_path[FILENAME_MAX];
            snprintf(file_path, FILENAME_MAX, "./%s%s/%s", userList[index].username, userList[index].dir, fileName);

            // Open the file for reading
            children.file = fopen(file_path, "rb");
            if (!children.file) {
                perror("Failed to open file");
                send_msg(children.command_fd, "550 File not found.\n");
                closeChild(SIGTERM);
            }

            printf("File okay, beginning data conenctions \n");

            // Open data connection
            if((children.data_fd = open_data_connection(userList[index].addr, userList[index].port)) < 0){
                send_msg(children.command_fd, "425 Can't open data connection.\n");
                closeChild(SIGTERM);
            }

            // Send file data
            char file_buffer[BUFFER_SIZE];
            int bytes_read;
            while ((bytes_read = fread(file_buffer, 1, BUFFER_SIZE, children.file)) > 0) {
                if(send(children.data_fd, file_buffer, bytes_read, 0) < 0){
                    perror("Send error");
                    send_msg(children.command_fd, "426 Connection closed; transfer aborted.\n");
                    closeChild(SIGTERM);
                }
            }

            fclose(children.file);
            close(children.data_fd);

            // Send transfer completion reply
            send_msg(children.command_fd, "226 Transfer complete.\n");
            closeChild(SIGTERM);
        }
    }
}

void listCom(User* userList, int index, int fd, char *buffer){
    if(!userList[index].auth){
        send_msg(fd, "530 Not logged in.\n");
        return;
    } else if(userList[index].addr.s_addr == 0 || userList[index].port == 0) {
        send_msg(fd, "425 Use PORT or PASV first.\n");
        return;
    } else {
        // Send preliminary reply
        send_msg(fd, "150 Opening data connection.\n");

        int pid = fork();
        if(pid < 0){
            perror("fork error");
            exit(1);
        }
        if(pid == 0){
            // Child process
            signal(SIGTERM, closeChild);
            children.command_fd = fd;

            printf("File okay, beginning data conenctions \n");

            // Open data connection
            if((children.data_fd = open_data_connection(userList[index].addr, userList[index].port)) < 0){
                send_msg(children.command_fd, "425 Can't open data connection.\n");
                closeChild(SIGTERM);
            }

            // Generate directory listing
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "ls -1 ./%s%s", userList[index].username, userList[index].dir);

            FILE *ls = popen(cmd, "r");
            if (!ls) {
                perror("Failed to run ls command");
                send_msg(children.command_fd, "550 Failed to list directory.\n");
                closeChild(SIGTERM);
            }

            // Read the output of the ls command and send it over the data connection
            char ls_buffer[BUFFER_SIZE];
            size_t bytes_read;
            while ((bytes_read = fread(ls_buffer, 1, sizeof(ls_buffer), ls)) > 0) {
                if (send(children.data_fd, ls_buffer, bytes_read, 0) < 0) {
                    perror("Send error");
                    send_msg(children.command_fd, "426 Connection closed; transfer aborted.\n");
                    pclose(ls);
                    closeChild(SIGTERM);
                }
            }

            pclose(ls);
            close(children.data_fd);

            // Send transfer completion reply
            send_msg(children.command_fd, "226 Transfer complete.\n");
            closeChild(SIGTERM);
        }
    }
}


void send_msg(int fd, char* msg) {
	send(fd, msg, strlen(msg)+1, 0);
}

int open_data_connection(struct in_addr client_addr, int client_port){
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr, ip_str, INET_ADDRSTRLEN);

    printf("Connecting to Client Transfer Socket... \n");

    int data_sd = socket(AF_INET, SOCK_STREAM, 0);
    if(data_sd < 0) {
        perror("Data socket creation failed");
        return -1;
    }

    // Allow the OS to assign an available port by binding to port 0
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(0); // Bind to any available port
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(data_sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        close(data_sd);
        return -1;
    }

    // Connect to client
    struct sockaddr_in client_socket_addr;
    memset(&client_socket_addr, 0, sizeof(client_socket_addr));
    client_socket_addr.sin_family = AF_INET;
    client_socket_addr.sin_port = htons(client_port);
    client_socket_addr.sin_addr = client_addr;

    if (connect(data_sd, (struct sockaddr*)&client_socket_addr, sizeof(client_socket_addr)) < 0) {
        perror("Connect error");
        close(data_sd);
        return -1;
    }
    printf("Connection Successful \n");
    return data_sd;
}

void closeChild(int sig) {
	// Clean up and exit
	if (sig == SIGTERM) {
		close(children.command_fd);
		close(children.data_fd);
		if (!children.file) {
			fclose(children.file);
			children.file = NULL;
		}
		exit(0);
	}
}