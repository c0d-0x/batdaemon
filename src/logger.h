#ifndef LOGGER_H
#define LOGGER_H
#include "filemond.h"
#include <pwd.h>
#include <signal.h>
#include <stdio.h>

typedef struct {
  char *p_event;
  char timedate[26];
  char *file_path; /* accessed file path*/
  char *Name;
  char *Umask;
  char *State;
  char *user_name;
} proc_info_t;

void cleanup_procinfo(proc_info_t *procinfo);
void proc_info(pid_t pid, char *buffer[], size_t buf_max);
proc_info_t *load_proc_info(char *buffer[]);
void writer_log(FILE *log_fd, proc_info_t *procinfo);
char *get_user(const uid_t uid);

#endif // !LOGGER_H
#define LOGGER_H
