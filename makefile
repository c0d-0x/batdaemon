# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -Wextra -Wformat-security 
# LIBS = 
MAIN = main.c

# Source and object files
CFILES = ./src/*.c
OBJFILES = *.o  

# Target binaries
BIN = ./bin/cruxfilemond

# Installation prefix (default /usr/local)
# PREFIX ?= /usr/local

# Installation directory
# DESTDIR = $(HOME)/.local/share/cruxfilemond

# Main target
all: $(BIN)

# Build executable
$(BIN): $(OBJFILES)
	$(CC) $(CFLAGS) $(LIBS) $(MAIN) -o $@ $^

# Compile source files
$(OBJFILES): $(CFILES)
	$(CC) $(CFLAGS) -c $^ 

# Install target
# install: $(BIN)
# 	sudo install -d $(PREFIX)/bin 
# 	install -d $(DESTDIR)
# 	sudo install -m 0755 $(BIN) $(PREFIX)/bin

# Phony target (no actual command)
.PHONY: clean uninstall all test

# Clean target
clean:
	rm -f *.o $(BIN)

# Uninstall target
# uninstall:
# 	rm -rf $(DESTDIR)
# 	sudo rm $(PREFIX)/bin/cruxfilemond

