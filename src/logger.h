#ifndef LOGGER_H
#define LOGGER_H
#include "filemond.h"
#include <pwd.h>
#include <signal.h>
#include <stdio.h>

typedef struct {
  char *p_event;
  char *file_path; /* accessed file path*/
  char *Name;
  size_t Umask;
  char *State;
  char *user_name;
} proc_info_t;

void proc_info(pid_t pid, char *buffer[], size_t buf_max);
proc_info_t *load_proc_info(char *buffer[]);
size_t writer_log(const int log_fd, proc_info_t *procinfo);
char *get_user(const uid_t uid);

#endif // !LOGGER_H
#define LOGGER_H
