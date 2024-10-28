#ifndef CLIENT_H
#define CLIENT_H

#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

typedef struct{
    FILE* file;
    int command_fd;
    int data_fd;
    int data_listen_fd;
} ChildP;

void handleCommand(int server_fd);
void handleMessage(int server_fd);
void storCom(int server_fd, char* buffer, int data_listen_fd);
int portCom(int server_fd);
void retrCom(int server_fd, char* buffer);
void listCom(int server_fd, char* buffer);
void handle_local_list(const char *args);
void handle_local_cwd(char *dir);
void handle_local_pwd(const char *args);
void closeChild(int sig);
void send_msg(int fd, char* msg);

#endif
