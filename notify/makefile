# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -Wextra -Wformat-security -pedantic
CLIBNOTFY = `pkg-config --cflags --libs libnotify`
# LIBS = 
MAIN = test.notify.c

# Source and object files
CFILES = notify.c
OBJFILES = notify.o  

# Target binaries
BIN = notest_notify


# Main target
all: $(BIN)
	@echo "Build Complete..."


# Build executable
$(BIN): $(OBJFILES)
	$(CC) $(CFLAGS) $(CLIBNOTFY) $(MAIN) -o $@ $^

# Compile source files
$(OBJFILES): $(CFILES)
	$(CC) $(CFLAGS) $(CLIBNOTFY) -c $^ 

# Phony target (no actual command)
.PHONY: clean all test

# Clean target
clean:
	rm *.o $(BIN) 


