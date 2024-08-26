#ifndef DAEMON_H

#define DAEMON_H
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/fanotify.h>
#include <linux/limits.h>
#include <poll.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define MAX_WATCH 200
#define CUSTOM_ERR (-1)
#define CONFIG_FILE "cf.config"
#define LOG_FILE "cf.log"
#define LOCK_FILE "cf.lock"
#define F_IS_DIR 1
#define F_NT_FND -1
#define F_IS_FILE 0

// create a file to save files and dirs currently being watched.
typedef struct {
  size_t F_TYPE;
  char *path;
} watch_t;

typedef struct {
  size_t watchlist_len;
  watch_t watchlist[MAX_WATCH];
} config_t;

config_t *load_config_file(char *file_Path);
void config_obj_cleanup(config_t *config_obj);
void fan_event_handler(int fan_fd, FILE *fp_log);
size_t check_lock(char *path_lock);
#endif
