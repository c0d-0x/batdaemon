#ifndef LOGGER_H
#define LOGGER_H
#include "filemond.h"
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
typedef enum { NAME = 0, UMASK, PPID, STATE, UID } proc_info_enum;
typedef struct {
  char *Name;
  char *Umask;
  char status;
  char *user_name;
  char *group_name;
  int ppid;
} proc_info_t;

void proc_info(pid_t pid, char *buffer[], size_t buf_max);
size_t writer_log(const int log_fd, const char *watched_path, proc_info_t *);
size_t get_user_group(const uid_t uid, const gid_t gid, proc_info_t *);

#endif // !LOGGER_H
#define LOGGER_H
