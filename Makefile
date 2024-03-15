CFLAGS ?= -pipe -Wall -Wextra -O3
BINDIR ?= /usr/local/bin

jumpns: jumpns.c
	$(CC) $(CFLAGS) $^ -o $@

install: jumpns
	install -m 0755 -t $(BINDIR) jumpns
	setcap CAP_SYS_ADMIN+ep $(BINDIR)/jumpns

clean:
	$(RM) jumpns

.PHONY: install clean
.SUFFIXES:
