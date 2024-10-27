CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

SRCS = serverEx.c clientEx.c ftp_commands.c util.c
OBJS = $(SRCS:.c=.o)
TARGETS = server client

all: $(TARGETS)

server: serverEx.o ftp_commands.o util.o
	$(CC) $(LDFLAGS) -o $@ $^

client: clientEx.o ftp_commands.o util.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all clean