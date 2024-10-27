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
    char buffer[BUFFER_SIZE] = {0};
    char cwd[BUFFER_SIZE];

    // Check if dir is NULL or empty
    if (dir == NULL || *dir == '\0') {
        snprintf(buffer, BUFFER_SIZE, "501 Syntax error in parameters or arguments.\r\n");
        send(client_sock, buffer, strlen(buffer), 0);
        return;
    }

    // Strip leading spaces and trailing newline
    while (*dir == ' ') dir++;
    strip_newline(dir);

    // Handle '..' for parent directory
    if (strcmp(dir, "..") == 0) {
        if (chdir("..") == 0) {
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                snprintf(buffer, BUFFER_SIZE, "250 Directory successfully changed to %s\r\n", cwd);
            } else {
                snprintf(buffer, BUFFER_SIZE, "550 Failed to get current directory: %s\r\n", strerror(errno));
            }
        } else {
            snprintf(buffer, BUFFER_SIZE, "550 Failed to change directory: %s\r\n", strerror(errno));
        }
    } else if (chdir(dir) == 0) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            snprintf(buffer, BUFFER_SIZE, "250 Directory successfully changed to %s\r\n", cwd);
        } else {
            snprintf(buffer, BUFFER_SIZE, "550 Failed to get current directory: %s\r\n", strerror(errno));
        }
    } else {
        snprintf(buffer, BUFFER_SIZE, "550 Failed to change directory: %s\r\n", strerror(errno));
    }
    send(client_sock, buffer, strlen(buffer), 0);
}

void handle_pwd(int client_sock) {
    char buffer[BUFFER_SIZE];
    char cwd[BUFFER_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(buffer, BUFFER_SIZE, "257 \"%s\" is the current directory\r\n", cwd);
    } else {
        snprintf(buffer, BUFFER_SIZE, "550 Failed to get current directory: %s\r\n", strerror(errno));
    }
    send(client_sock, buffer, strlen(buffer), 0);
}

void handle_local_cwd(char *dir) {
    char cwd[BUFFER_SIZE];

    // Check if dir is NULL or empty
    if (dir == NULL || *dir == '\0') {
        printf("Error: No directory specified.\n");
        return;
    }

    // Strip leading spaces
    while(*dir == ' ') dir++;

    if (strcmp(dir, "..") == 0) {
        // Change to parent directory
        if (chdir("..") == 0) {
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Local directory changed to %s\n", cwd);
            } else {
                printf("Error getting current directory: %s\n", strerror(errno));
            }
        } else {
            printf("Failed to change to parent directory: %s\n", strerror(errno));
        }
    } else if (*dir == '~') {
        // Change to home directory
        const char *home = getenv("HOME");
        if (home != NULL && chdir(home) == 0) {
            printf("Local directory changed to %s\n", home);
        } else {
            printf("Failed to change to home directory: %s\n", strerror(errno));
        }
    } else {
        // Change to specified directory
        if (chdir(dir) == 0) {
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Local directory changed to %s\n", cwd);
            } else {
                printf("Error getting current directory: %s\n", strerror(errno));
            }
        } else {
            printf("Failed to change directory: %s\n", strerror(errno));
        }
    }
}

void handle_local_pwd() {
    char cwd[BUFFER_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Local current directory: %s\n", cwd);
    } else {
        printf("Error getting current directory: %s\n", strerror(errno));
    }
}

void handle_local_list() {
    char command[BUFFER_SIZE];
    snprintf(command, BUFFER_SIZE, "ls -l");
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return;
    }

    char output[BUFFER_SIZE];
    while (fgets(output, BUFFER_SIZE, fp) != NULL) {
        printf("%s", output);
    }

    int status = pclose(fp);
    if (status == -1) {
        printf("Error closing pipe: %s\n", strerror(errno));
    } else if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) {
            printf("Command exited with status %d\n", exit_status);
        }
    } else if (WIFSIGNALED(status)) {
        printf("Command killed by signal %d\n", WTERMSIG(status));
    }
}
