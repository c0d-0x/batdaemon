#ifndef INOTIFY_H_

#define INOTIFY_H_
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "filemond.h"

int init_inotify(char* file_path);
config_t* inotify_event_handler(int inotify_fd, int config_fd,
                                config_t* (*handler)(int config_fd));

#endif  // !INOTIFY_H_
