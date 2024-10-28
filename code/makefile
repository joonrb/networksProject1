CC = gcc
CFLAGS = -w -g  
CLIENT_SRC = clientEx.c
SERVER_SRC = serverEx.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_BIN = client
SERVER_BIN = server

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ) client.h
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJ)

$(SERVER_BIN): $(SERVER_OBJ) server.h
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT_BIN) $(SERVER_BIN)

.PHONY: all clean

the make file