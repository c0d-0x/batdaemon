#ifndef DAEMON_H
#define DAEMON_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

char *load_file(char *file_Path);
#endif
