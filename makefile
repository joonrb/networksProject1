CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread

SERVER_SRC = server_yumi_revised.c util.c
CLIENT_SRC = client_yumi_revised.c util.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: server_exec client_exec

server_exec: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client_exec: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f server_exec client_exec *.o

.PHONY: all clean