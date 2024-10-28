CC=gcc
CFLAGS=-Wall -g
SRCDIR=code
EXECUTABLES=serverEx clientEx

all: $(EXECUTABLES)

serverEx: $(SRCDIR)/serverEx.o
	$(CC) $(CFLAGS) -o $@ $^

clientEx: $(SRCDIR)/clientEx.o
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(SRCDIR)/*.o $(EXECUTABLES)

.PHONY: all clean
