#ifndef LOGGER_H
#define LOGGER_H
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <time.h>

#include "filemond.h"
#include "json_gen.h"

/**
 * Custom stack for temporally
 * save procsss info before writing to our log file
 * */

typedef struct node {
  json_obj_t *data;
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
void cleanup_procinfo(json_obj_t *json_obj);
int push_stk(cus_stack_t **head, json_obj_t *data);
cus_stack_t *pop_stk(cus_stack_t **head);
void proc_info(pid_t pid, char *buffer[], size_t buf_max);
json_obj_t *tokenizer(char *buffer[]);
char *get_user(const uid_t uid);

#endif  // !LOGGER_H
