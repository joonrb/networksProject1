# networksProject1


## File Structure

- `code/`: Contains the source code for the client and server.
  - `clientEx.c`: Client implementation.
  - `serverEx.c`: Server implementation.
  - `makefile`: Makefile for building the project.
  - `client.h`: Header file for the client.
  - `server.h`: Header file for the server.

## Key Features

- **Concurrent Connections:** The server supports multiple simultaneous client connections using `select()` and `fork()`.
- **Authentication:** Users must authenticate using `USER` and `PASS` commands.
- **Command Support:** Includes commands like `CWD`, `PWD`, `LIST`, `RETR`, `STOR`, and `QUIT`.
- **Local Commands:** The client supports local directory commands prefixed with `!`.

## Setup and Compilation

1. **Compile the Project:**
   - Use the provided `Makefile` to compile the client and server:
     ```bash
     make
     ```

2. **Run the Server:**
   - Start the server:
     ```bash
     ./ftp_server
     ```

3. **Run the Client:**
   - Start the client:
     ```bash
     ./ftp_client
     ```

## Usage

1. **Authentication:**
   - `USER <username>`
   - `PASS <password>`

2. **Directory Navigation:**
   - `CWD <directory>`
   - `PWD`

3. **File Transfer:**
   - `RETR <filename>`
   - `STOR <filename>`

4. **Directory Listing:**
   - `LIST`

5. **Session Management:**
   - `QUIT`

6. **Local Commands:**
   - `!CWD <directory>`
   - `!PWD`
   - `!LIST`

## Testing Commands

To ensure the FTP server and client are functioning correctly, use the following test commands:

- Authenticate with `USER` and `PASS`.
- Navigate directories with `CWD` and `PWD`.
- Transfer files using `RETR` and `STOR`.
- List directory contents with `LIST`.
- Use local commands with `!CWD`, `!PWD`, and `!LIST`.

