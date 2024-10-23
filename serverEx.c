#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 21
#define BUFFER_SIZE 1024

// Structure to hold user credentials
typedef struct{
    char username[50];
    char password[50];
}Login;

Login list[100];  // Adjust size based on expected number of users
int userNum = 0;

// Load username and password from users.csv
void loadLogin(){
    FILE *file = fopen("users.csv", "r");
    if(!file){
        perror("Failed to open file");
        exit(1);
    }
    while(fscanf(file, "%49[^,],%49s\n", list[userNum].username, list[userNum].password) == 2){
        userNum++;
    }
    fclose(file);
}

bool checkUser(char *user){
    for(int i = 0; i < userNum; i++){
        if(strcmp(user, list[i].username) == 0){
            return true;
        }
    }
    return false;
}

bool checkPass(char *user, char *pass){
    for(int i = 0; i < userNum; i++){
        if(strcmp(user, list[i].username) == 0 && strcmp(pass, list[i].password) == 0){
            return true;
        }
    }
    return false;
}

// Function declarations
void process_commands(int client_sock);

int main(){
    loadLogin();

    int server_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        perror("Socket creation failed");
        return 1;
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for clients
    if(listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("FTP Server is listening on port %d\n", PORT);

    // Accept clients
    while(1){
        client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if(client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Connection established with %s\n", inet_ntoa(client_addr.sin_addr));
        process_commands(client_sock);
        close(client_sock);
    }

    close(server_fd);
    return 0;
}

// Function to process commands from client
void process_commands(int client_sock){
    char buffer[BUFFER_SIZE];
    int bytes_read;
    char currentUser[50] = {0};
    bool userAuth = false;
    bool passAuth = false;

    // Send welcome message
    send(client_sock, "220 Welcome to Simple FTP\n", 26, 0);

    while((bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0)) > 0){
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);

        if(strncmp("USER", buffer, 4) == 0){
            strcpy(currentUser, buffer + 5);
            if(checkUser(currentUser)){
                userAuth = true;
                send(client_sock, "331 Username OK, need password.\n", 31, 0);
            }
            else{
                send(client_sock, "530 Not logged in.\n", 24, 0);
            }
        }
        else if(strncmp("PASS", buffer, 4) == 0 && userAuth){
            if(checkPass(currentUser, buffer + 5)){
                passAuth = true;
                send(client_sock, "230 User logged in, proceed.\n", 31, 0);
            }
            else{
                send(client_sock, "530 Not logged in.\n", 24, 0);
            }
        }
        else if(userAuth && passAuth && strncmp("STOR", buffer, 4) == 0){
            //Code for STOR command
        }
        else if(userAuth && passAuth && strncmp("RETR", buffer, 4) == 0){
            //Code for RETR command
        }
        else if(userAuth && passAuth && strncmp("LIST", buffer, 4) == 0){
            //Code for LIST command
        }
        else if(userAuth && passAuth && strncmp("!LIST", buffer, 4) == 0){
            //Code for !LIST command
        }
        else if(userAuth && passAuth && strncmp("CWD", buffer, 4) == 0){
            //Code for CWD command
            char *dir = buffer + 4;
            while(*dir == ' ') dir++; //skip spaces
            if(chdir(dir) == 0){
                char cwd[BUFFER_SIZE];
                getcwd(cwd, sizeof(cwd));
                snprintf(buffer, BUFFER_SIZE, "200 directory changed to")
            }
        }
        else if(userAuth && passAuth && strncmp("!CWD", buffer, 4) == 0){
            //Code for !CWD command
        }
        else if(userAuth && passAuth && strncmp("PWD", buffer, 4) == 0){
            //Code for PWD command
        }
        else if(userAuth && passAuth && strncmp("!PWD", buffer, 4) == 0){
            //Code for !PWD command
        }
        else {
            // If neither username nor password is validated, prompt for login
            send(client_sock, "530 Please login with USER and PASS.\n", 36, 0);
        }

        if(strncmp("QUIT", buffer, 4) == 0){
            send(client_sock, "221 Goodbye\n", 12, 0);
            break;
        }
    }
}
