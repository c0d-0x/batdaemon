# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -Wextra -Wformat-security -pedantic
# LIBS = 
CRX_MSG = cruxfilemond_ipc
MAIN = main.c

# Source and object files
CFILES = ./src/*.c
OBJFILES = *.o  

# Target binaries
BIN = ./bin

# Installation prefix (default /usr/local)
# PREFIX ?= /usr/local

# Installation directory
# DESTDIR = $(HOME)/.local/share/cruxfilemond

# Main target
all: $(BIN)/cruxfilemond
	$(CC) $(CFLAGS) $(CRX_MSG).c -o $(BIN)/$(CRX_MSG) 
	@echo "Build Complete..."


# Build executable
$(BIN)/cruxfilemond: $(OBJFILES)
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
	rm *.o $(BIN)/* 

# Uninstall target
# uninstall:
# 	rm -rf $(DESTDIR)
# 	sudo rm $(PREFIX)/bin/cruxfilemond

