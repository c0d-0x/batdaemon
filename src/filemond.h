#ifndef DAEMON_H

#define DAEMON_H
#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
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

typedef enum { NAME = 0, UMASK, PPID, STATE, UID } proc_info_enum;
typedef struct {
  char *Name;
  char *Umask;
  char status;
  int Uid;
  int ppid;
} proc_info_t;

config_t *load_config_file(char *file_Path);
void config_obj_cleanup(config_t *config_obj);
static int write_log(int, char *, char **);
void fan_event_handler(int fan_fd);
size_t check_lock(char *path_lock);
#endif
