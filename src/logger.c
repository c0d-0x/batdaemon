#include "logger.h"
#include "filemond.h"

void proc_info(pid_t pid, char *buffer[], size_t buf_max) {
  char procfd_path[32] = {0x0};
  char buf_temp[64] = {0x0};
  char *tok = NULL;
  FILE *proc_fd = NULL;
  size_t index_n, i = 0;

  snprintf(procfd_path, sizeof(procfd_path), "/proc/%d/status", pid);
  if (access(procfd_path, F_OK) == 0) {
    if ((proc_fd = fopen(procfd_path, "r")) == NULL) {
      syslog(LOG_ERR, "Failed to open proc_fd: %s", strerror(errno));
      return;
    }

    while (fgets(buf_temp, sizeof(buf_temp), proc_fd) != NULL && i < buf_max) {
      if ((tok = strchr(buf_temp, '\n')) != NULL) {
        index_n = tok - buf_temp;
        buf_temp[index_n] = '\0';
      }
      buffer[i] = strdup(buf_temp);
      i++;
    }
    fclose(proc_fd);
  }
}

char *get_user(const uid_t uid) {

  struct passwd *pws;
  if ((pws = getpwuid(uid)) != NULL)
    return (pws->pw_name);

  return NULL;
}

proc_info_t *load_proc_info(char *buffer[]) {
  size_t i = 0;
  char *saveptr;
  char *token;
  proc_info_t *proc_info_struct = NULL;
  if ((proc_info_struct = calloc(0x1, sizeof(proc_info_t))) == NULL) {
    syslog(LOG_ERR, "Calloc Failed: %s", strerror(errno));
    return NULL;
  }

  while (buffer[i] != NULL && i < 11) {
    token = strtok_r(buffer[i], ":\t\r ", &saveptr);
    if (token == NULL) {
      syslog(LOG_ERR, "Failed to load_proc_info\n");
      exit(CUSTOM_ERR);
    }

    // printf(" token: %s\n", token);
    if (strncmp(token, "Name", sizeof("Name")) == 0) {
      token = strtok_r(NULL, " ", &saveptr);
      proc_info_struct->Name = strdup(token);
    }

    if (strncmp(token, "Umask", sizeof("Umask")) == 0) {
      token = strtok_r(NULL, " ", &saveptr);
      proc_info_struct->Umask = strdup(token);
    }

    if (strncmp(token, "State", sizeof("State")) == 0) {
      token = strtok_r(NULL, " ", &saveptr);
      proc_info_struct->State = strdup(saveptr);
    }

    if (strncmp(token, "Uid", sizeof("Uid")) == 0) {
      token = strtok_r(NULL, " ", &saveptr);
      proc_info_struct->user_name = strdup(get_user(atoi(token)));
    }
    free(buffer[i]);
    i++;
  }
  return proc_info_struct;
}

void cleanup_procinfo(proc_info_t *procinfo) {
  if (procinfo != NULL) {
    if (procinfo->user_name != NULL)
      free(procinfo->user_name);
    if (procinfo->Name != NULL)
      free(procinfo->Name);
    if (procinfo->State != NULL)
      free(procinfo->State);
    if (procinfo->Umask != NULL)
      free(procinfo->Umask);
  }
}

int push_stk(cus_stack_t **head, proc_info_t *data) {

  cus_stack_t *node = NULL;
  if ((node = (cus_stack_t *)malloc(sizeof(cus_stack_t))) == NULL) {
    perror("Malloc Failed");
    return -1;
  }

  node->data = data;
  node->next = (*head);
  (*head) = node;
  return 0;
}

cus_stack_t *pop_stk(cus_stack_t **head) {

  if ((*head) == NULL) {
    return NULL;
  }

  cus_stack_t *node = (*head);
  (*head) = (*head)->next;
  node->next = NULL;
  return node;
}

void get_locale_time(char *buf) {
  if (buf == NULL)
    return;
  struct tm tm = *localtime(&(time_t){time(NULL)});
  asctime_r(&tm, buf);
  buf[strnlen(buf, 26) - 1] = '\0';
}

void writer_log(FILE *log_fd, proc_info_t *procinfo) {
  get_locale_time(procinfo->timedate);
  fprintf(log_fd, "[ %s ]\n", procinfo->timedate);
  fprintf(log_fd, "File: %s\n", procinfo->file_path);
  fprintf(log_fd, "Event: %s\n", procinfo->p_event);
  fprintf(log_fd, "Effective Process: %s\n", procinfo->Name);
  fprintf(log_fd, "Effective Username: %s\n", procinfo->user_name);
  fprintf(log_fd, "Effective Process Umask: %s\n", procinfo->Umask);
  fprintf(log_fd, "Effective Process State: %s\n", procinfo->State);
  return;
}
