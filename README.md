Here's the content formatted as a Markdown (.md) file:

# Simplified FTP Application Protocol Project

## Project Overview

This project involves building a simplified version of the FTP application protocol, consisting of two separate programs: an FTP client and an FTP server. The FTP server is responsible for maintaining FTP sessions and providing file access, while the FTP client is split into two components: an FTP user interface and an FTP client to make requests to the FTP server[1].

## Key Requirements

1. The FTP server must be started first and support concurrent connections.
2. The server must handle multiple simultaneous requests from different or the same client.
3. Use `select()` and `fork()` functions in C for handling connections and requests.
4. `fork()` should be used for resource-intensive requests/commands with data transfer.
5. `select()` is sufficient for control connection making and simple commands[1].

## FTP Protocol Overview

The FTP protocol uses separate connections for session control and data transfers:

- Control channel: Client connects from a random unprivileged port (N > 1024) to server port 21.
- Data channel: For each data transfer, the client sends a PORT N+1 command to the server.
- The server connects from port 20 to the client-specified port for data transfer.
- Only active FTP mode needs to be implemented[1].

## FTP Commands and Replies

The client should provide a prompt `ftp>` for user input. The following commands must be implemented:

### Server Commands

- `PORT h1,h2,h3,h4,p1,p2`: Specify client IP and port for data channel.
- `USER username`: Identify user for login.
- `PASS password`: Authenticate with user password.
- `STOR filename`: Upload a file to the server.
- `RETR filename`: Download a file from the server.
- `LIST`: List files in the current server directory.
- `CWD foldername`: Change current server directory.
- `PWD`: Display current server directory.
- `QUIT`: End FTP session and close connection[1].

### Client Commands

- `!LIST`: List files in the current client directory.
- `!CWD foldername`: Change current client directory.
- `!PWD`: Display current client directory[1].

## Authentication

- User credentials should be stored in a `users.txt` file.
- The file must contain an entry for username "bob" with password "donuts"[1].

## Data Transfer

- RETR, STOR, and LIST commands trigger data transfer.
- Server replies with "150 File status okay; about to open data connection."
- Stream transfer mode is used.
- End of file is indicated by closing the data connection[1].

## Error Handling

The server must respond with appropriate error messages:

- "202 Command not implemented." for invalid commands.
- "530 Not logged in." for unauthenticated requests.
- "550 No such file or directory." for invalid filenames or directories.
- "503 Bad sequence of commands." for invalid command sequences[1].

## Example FTP Session

```
220 Service ready for new user.
ftp> USER bob
331 Username OK, need password.
ftp> PASS donuts
230 User logged in, proceed.
ftp> CWD test
200 directory changed to /Users/bob/test
ftp> RETR vanilla_donut.txt
200 PORT command successful.
150 File status okay; about to open. data connection.
226 Transfer completed.
ftp> LIST
200 PORT command successful.
150 File status okay; about to open. data connection.
vanilla_donut.txt
choco_donut.txt
226 Transfer completed.
ftp> QUIT
221 Service closing control connection.
```

[1]

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/1703058/9899e49a-eea5-4c14-9512-5afc02f0cf87/paste.txt