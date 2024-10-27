#ifndef SERVER_H
#define SERVER_H

#define PORT 2121
#define BUFFER_SIZE 1024
#define NUM_OF_USERS 3
#define MAX_NAME_SIZE 10
#define MAX_CONNECT 30

typedef struct{
    int userfd;
    char* username;
    int auth;
    int port;
    char* usr_dir;
    struct in_addr addr;
    char dir[FILENAME_MAX];
    char command_buffer[BUFFER_SIZE];
    int buffer_len;
} User;

typedef struct{
    char username[MAX_NAME_SIZE];
    char password[MAX_NAME_SIZE];
}Login;

typedef struct{
    FILE* file;
    int command_fd;
    int data_fd;
} ChildP;

int userNum = 0;

void loadUser(Login* db);
void open_session(int server_fd, fd_set* allsocket, int* max_socket_so_far);
void checkUser(User* userList, int index, int fd, char *buffer);
void checkPass(User* userList, int index, int fd, char *buffer);
void portCom(User* userList, int index, int fd, char *buffer);
void storCom(User* userList, int index, int fd, char *buffer);
void retrCom(User* userList, int index, int fd, char *buffer);
void listCom(User* userList, int index, int fd, char *buffer);
int open_data_connection(struct in_addr client_addr, int client_port);
void handleCommand(int fd, fd_set* allsocket, int* max_socket_so_far, User* userList);
void send_msg(int fd, char* msg);
void closeChild(int sig);

#endif