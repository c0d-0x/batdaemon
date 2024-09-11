#ifndef LOGGER_H
#define LOGGER_H
#include "filemond.h"
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <time.h>

typedef struct {
  char *p_event;
  char timedate[26];
  char *file_path; /* accessed file path*/
  char *Name;
  char *Umask;
  char *State;
  char *user_name;
} proc_info_t;

/**
 * Custom stack for temporally
 * save procsss info before writing to our log file
 * */

typedef struct node {
  proc_info_t *data;
  struct node *next;
} cus_stack_t;

/**
 * @brief Frees the dynamically allocated memory in the proc_info_t structure.
 *
 * This function releases the memory allocated for the string fields in the
 * proc_info_t structure. It is important to call this function after using the
 * proc_info_t structure to prevent memory leaks.
 *
 * @param procinfo A pointer to the proc_info_t structure whose memory needs to
 * be freed.
 */
void get_locale_time(char *buf);
void cleanup_procinfo(proc_info_t *procinfo);
int push_stk(cus_stack_t **head, proc_info_t *data);
cus_stack_t *pop_stk(cus_stack_t **head);
void proc_info(pid_t pid, char *buffer[], size_t buf_max);
proc_info_t *load_proc_info(char *buffer[]);
void writer_log(FILE *log_fd, proc_info_t *procinfo);
char *get_user(const uid_t uid);

#endif // !LOGGER_H
#define LOGGER_H
