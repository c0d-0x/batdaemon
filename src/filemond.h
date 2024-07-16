#ifndef DAEMON_H

#define DAEMON_H
#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_WATCH 200
#define CUSTOM_ERR -1
#define CONFIG_FILE "cf.config"
#define LOG_FILE "cf.log"
#define LOCK_FILE "cf.lock"

// create a file to save files and dirs currently being watched.
typedef struct {
  size_t watchlist_len;
  char *watchlist[MAX_WATCH];
} config_t;

config_t *load_config_file(char *file_Path);
void config_obj_cleanup(config_t *config_obj);
char *write_log(void);
size_t check_lock(char *path_lock);
#endif
