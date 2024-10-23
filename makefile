CC = gcc
CFLAGS = -Wall -Wextra -g
SERVER = ftpserver
CLIENT = ftpclient
SERVER_SRC = serverEx.c util.c
CLIENT_SRC = clientEx.c
HEADERS = util.h

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC)

$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC)

clean:
	rm -f $(SERVER) $(CLIENT)

.PHONY: all clean
