# project1

**Key Requirements:**

1. **FTP Server:**
    - Must support concurrent connections using `select()` and `fork()`.
    - Use `select()` for handling control connections and simple commands.
    - Use `fork()` for resource-intensive commands involving data transfer (e.g., `RETR`, `STOR`, `LIST`).
    - Should read user credentials from a `users.txt` file upon launch.
    - Must handle authentication (`USER` and `PASS` commands).
2. **FTP Client:**
    - Split into two components: user interface and client logic.
    - Provides a command prompt (`ftp>`) for user input.
    - Should automatically handle the `PORT` command before data transfer commands.
    - Executes local commands starting with `!` (e.g., `!LIST`, `!PWD`) using the `system()` function.
3. **FTP Commands to Implement:**
    - **Control Commands:**
        - `USER username`
        - `PASS password`
        - `CWD foldername`
        - `PWD`
        - `QUIT`
    - **Data Transfer Commands:**
        - `PORT h1,h2,h3,h4,p1,p2`
        - `RETR filename`
        - `STOR filename`
        - `LIST`
    - **Local Commands (executed on the client side):**
        - `!CWD foldername`
        - `!PWD`
        - `!LIST`
4. **Error Handling:**
    - Invalid commands should result in a `202 Command not implemented.` response.
    - Unauthorized access attempts should result in a `530 Not logged in.` response.
    - Invalid filenames or directories should result in a `550 No such file or directory.` response.
    - Incorrect command sequences should result in a `503 Bad sequence of commands.` response.

**Approach to Implementation:**

1. **Server Side:**
    - **Initialization:**
        - Start by creating a socket that listens on port 21 (the control connection port).
        - Load user credentials from `users.txt`.
    - **Handling Connections:**
        - Use `select()` to monitor multiple client connections on the control channel.
        - When a data transfer command is received (`RETR`, `STOR`, `LIST`):
            - Use `fork()` to handle the data transfer in a child process.
            - The parent process continues to handle the control connection.
    - **Data Transfer:**
        - For active FTP mode, the server connects back to the client on the port specified in the `PORT` command (originating from port 20).
        - Implement the data transfer in stream mode.
        - Close the data connection to signify the end of the transfer.
    - **Authentication:**
        - Require successful `USER` and `PASS` commands before processing other commands.
        - Maintain session state to track authenticated users.
2. **Client Side:**
    - **User Interface:**
        - Display a prompt (`ftp>`) and read user input.
        - Parse commands and handle local commands starting with `!` using `system()`.
    - **Control Connection:**
        - Establish a control connection to the server on port 21.
        - Handle server responses and display appropriate messages to the user.
    - **Data Connection:**
        - Before sending `RETR`, `STOR`, or `LIST` commands:
            - Send a `PORT` command with the client's IP and a dynamically chosen port (greater than 1024).
            - Start listening on that port for the server's data connection.
3. **Concurrency:**
    - Ensure the server can handle multiple clients simultaneously.
    - Use `select()` to handle multiple control connections without blocking.
    - Use `fork()` to handle data transfers concurrently.

**Sample Code Structure:**

Below is a high-level overview of how you might structure your code.

**Server (`ftp_server.c`):**

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// Include necessary socket headers

#define CONTROL_PORT 21
#define DATA_PORT 20

void handle_client(int client_sock);
void handle_command(int client_sock, char *command);
void handle_data_transfer(int client_sock, char *command);

int main() {
    // Create a socket and bind to CONTROL_PORT
    // Load user credentials from users.txt
    // Use listen() and accept() to handle incoming connections
    // Use select() to monitor multiple client sockets
    // For each client socket, read commands and process them
    return 0;
}

```

**Client (`ftp_client.c`):**

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// Include necessary socket headers

#define SERVER_PORT 21

void ftp_prompt(int control_sock);

int main(int argc, char *argv[]) {
    // Establish a control connection to the server
    // Call ftp_prompt() to handle user input
    return 0;
}

void ftp_prompt(int control_sock) {
    char command[256];
    while (1) {
        printf("ftp> ");
        fgets(command, sizeof(command), stdin);
        // Trim newline and parse the command
        // Handle local commands starting with '!'
        // Send other commands to the server via control_sock
        // For data transfer commands, handle PORT and data connection
    }
}

```

**Key Functions to Implement:**

- **Parsing the `PORT` Command:**
    
    ```c
    void parse_port_command(char *arg, char *ip, int *port) {
        int h1, h2, h3, h4, p1, p2;
        sscanf(arg, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
        sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
        *port = (p1 << 8) | p2;
    }
    
    ```
    
- **Sending the `PORT` Command:**
    
    ```c
    void send_port_command(int control_sock, int data_port) {
        char command[256];
        // Get client's IP address
        // Break IP and port into h1,h2,h3,h4,p1,p2
        sprintf(command, "PORT %d,%d,%d,%d,%d,%d\\r\\n", h1, h2, h3, h4, p1, p2);
        send(control_sock, command, strlen(command), 0);
    }
    
    ```
    

**Testing and Debugging:**

- **Start the Server First:**
    - Run your server program and ensure it's listening on port 21.
    - Verify that it's ready to accept connections.
- **Connect the Client:**
    - Run your client program and connect to the server.
    - Test authentication using the `USER` and `PASS` commands.
- **Execute Commands:**
    - Try simple commands like `PWD`, `CWD`, and `LIST`.
    - Test data transfer commands like `RETR` and `STOR` with various file sizes.
- **Concurrent Connections:**
    - Open multiple client instances to test concurrent connections.
    - Ensure that data transfers and control commands do not interfere with each other.

**Additional Tips:**

- **Signal Handling:**
    - Implement signal handlers to reap zombie processes created by `fork()`.
- **Resource Management:**
    - Ensure that all sockets are properly closed after use.
    - Handle errors gracefully and provide meaningful messages to the user.
- **Security Considerations:**
    - Since this is a simplified FTP implementation, you can skip implementing encryption (e.g., FTPS).
    - Be cautious with file paths to avoid directory traversal vulnerabilities.

**Reference Materials:**

- **RFC 959 - File Transfer Protocol:** Provides detailed specifications of FTP commands and responses.
- **Beej's Guide to Network Programming:** An excellent resource for socket programming in C.

**Example FTP Session Explanation:**

```
220 Service ready for new user.
ftp> USER bob
331 Username OK, need password.
ftp> PASS donuts
230 User logged in, proceed.

```

- The client connects to the server and receives a `220` greeting.
- The client sends `USER bob`, and the server asks for a password with `331`.
- The client sends `PASS donuts`, and the server logs the user in with `230`.

```
ftp> CWD test
200 directory changed to /Users/bob/test

```

- The client changes the working directory on the server to `test`.
- The server confirms with a `200` response.

```
ftp> RETR vanilla_donut.txt
200 PORT command successful.
150 File status okay; about to open data connection.
226 Transfer completed.

```

- The client initiates a file retrieval.
- The client sends a `PORT` command (handled automatically).
- The server acknowledges the `PORT` command with `200`.
- The server starts the data transfer (`150`) and completes it (`226`).

```
ftp> LIST
200 PORT command successful.
150 File status okay; about to open data connection.
vanilla_donut.txt
choco_donut.txt
226 Transfer completed.

```

- The client requests a directory listing.
- Similar to `RETR`, the `PORT` command is used.
- The server sends the directory listing over the data connection.

```
ftp> QUIT
221 Service closing control connection.

```

- The client ends the session, and the server closes the control connection.

---

Feel free to ask if you have specific questions about any part of the implementation, or if you need clarification on certain functions or concepts.