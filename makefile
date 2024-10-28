CC = gcc
CFLAGS = -w -g  
SRCDIR = code
CLIENT_SRC = $(SRCDIR)/clientEx.c
SERVER_SRC = $(SRCDIR)/serverEx.c
CLIENT_OBJ = $(SRCDIR)/clientEx.o
SERVER_OBJ = $(SRCDIR)/serverEx.o
CLIENT_BIN = clientEx
SERVER_BIN = serverEx

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ) $(SRCDIR)/client.h
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJ)

$(SERVER_BIN): $(SERVER_OBJ) $(SRCDIR)/server.h
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT_BIN) $(SERVER_BIN)

.PHONY: all clean