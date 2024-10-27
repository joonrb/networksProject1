#include "ftp_commands.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

// Utility functions
void strip_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
    }
}



// Server-side functions
void handle_cwd(int client_sock, char *dir) {
    char buffer[BUFFER_SIZE];
    char cwd[BUFFER_SIZE];
    char original_dir[BUFFER_SIZE];

    // Store original directory
    if (getcwd(original_dir, sizeof(original_dir)) == NULL) {
        snprintf(buffer, BUFFER_SIZE, "550 Failed to get current directory: %s\r\n", strerror(errno));
        send(client_sock, buffer, strlen(buffer), 0);
        return;
    }

    // Skip leading spaces
    while (dir && *dir == ' ') dir++;

    if (!dir || *dir == '\0') {
        snprintf(buffer, BUFFER_SIZE, "501 Syntax error in parameters. Usage: CWD <directory>\r\n");
        send(client_sock, buffer, strlen(buffer), 0);
        return;
    }

    // Try to change directory
    if (chdir(dir) == 0) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            // Verify we're still under the user's directory
            if (strstr(cwd, original_dir) == cwd) {
                snprintf(buffer, BUFFER_SIZE, "250 Directory successfully changed to %s\r\n", cwd);
            } else {
                // If outside user's directory, go back
                chdir(original_dir);
                snprintf(buffer, BUFFER_SIZE, "550 Access denied: Cannot leave user directory\r\n");
            }
        } else {
            chdir(original_dir);
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

void send_file_list(int client_sock) {
    DIR *d;
    struct dirent *dir;
    char buffer[BUFFER_SIZE];

    d = opendir(".");
    if (d) {
        send(client_sock, "150 Here comes the directory listing.\r\n", 
             strlen("150 Here comes the directory listing.\r\n"), 0);
        
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) { // Only list regular files
                snprintf(buffer, BUFFER_SIZE, "%s\r\n", dir->d_name);
                send(client_sock, buffer, strlen(buffer), 0);
            }
        }
        closedir(d);
        send(client_sock, "226 Directory send OK.\r\n", 
             strlen("226 Directory send OK.\r\n"), 0);
    } else {
        send(client_sock, "550 Failed to open directory.\r\n", 
             strlen("550 Failed to open directory.\r\n"), 0);
    }
}

// Client-side functions
void handle_local_cwd(char *dir) {
    // Skip leading spaces
    while (dir && *dir == ' ') dir++;

    if (!dir || *dir == '\0') {
        printf("501 Syntax error: Usage: !CWD <directory>\n");
        return;
    }

    // Handle home directory shortcut
    if (strcmp(dir, "~") == 0) {
        char *home = getenv("HOME");
        if (home && chdir(home) == 0) {
            char cwd[BUFFER_SIZE];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Local directory changed to %s\n", cwd);
            } else {
                printf("Error getting current directory: %s\n", strerror(errno));
            }
            return;
        }
    }

    if (chdir(dir) == 0) {
        char cwd[BUFFER_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Local directory changed to %s\n", cwd);
        } else {
            printf("Error getting current directory: %s\n", strerror(errno));
        }
    } else {
        printf("550 Failed to change directory: %s\n", strerror(errno));
    }
}

void handle_local_pwd(const char *args) {
    char cwd[BUFFER_SIZE];

    // Skip leading spaces
    while (args && *args == ' ') args++;

    if (args && *args != '\0') {
        printf("501 Syntax error: !PWD command doesn't accept arguments\n");
        return;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Local current directory: %s\n", cwd);
    } else {
        printf("Error getting current directory: %s\n", strerror(errno));
    }
}

void handle_local_list(const char *args) {
    // Skip leading spaces
    while (args && *args == ' ') args++;

    if (args && *args != '\0') {
        printf("501 Syntax error: !LIST command doesn't accept arguments\n");
        return;
    }

    FILE *fp = popen("ls -l", "r");
    if (!fp) {
        printf("Error executing ls command: %s\n", strerror(errno));
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    int status = pclose(fp);
    if (status == -1) {
        printf("Error closing pipe: %s\n", strerror(errno));
    }
}
