CC = gcc
CFLAGS = -Wall -Wextra -g -D_DEFAULT_SOURCE
LDFLAGS = 

# Source files
SERVER_SRCS = serverEx.c ftp_commands.c
CLIENT_SRCS = clientEx.c ftp_commands.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
DEPS = ftp_commands.h client.h server.h

.PHONY: all clean

all: server_exec client_exec

server_exec: $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

client_exec: $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) server_exec client_exec *~