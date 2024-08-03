#include "logger.h"
#include "filemond.h"
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void proc_info(pid_t pid, char *buffer[], size_t buf_max) {
  char procfd_path[32] = {0x0};
  char buf_temp[64] = {0x0};
  char *tok = NULL;
  FILE *proc_fd = NULL;
  size_t index_n, i = 0;

  snprintf(procfd_path, sizeof(procfd_path), "/proc/%d/status", pid);

  procfd_path[strlen(procfd_path)] = '\0';
  if (access(procfd_path, F_OK) == 0) {
    if ((proc_fd = fopen(procfd_path, "r")) == NULL) {
      perror("Failed to open proc_fd");
      return;
    }

    while (fgets(buf_temp, sizeof(buf_temp), proc_fd) > 0 && i < buf_max) {
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
  if ((proc_info_struct = calloc(sizeof(proc_info_t), 0x1)) == NULL) {
    perror("Calloc Failed");
    return NULL;
  }

  while (buffer[i] != NULL && i < 11) {
    token = strtok_r(buffer[i], ":", &saveptr);
    if (token == NULL) {
      errx(CUSTOM_ERR, "Failed to load_proc_info\n");
    }

    // printf(" token: %s\n", token);
    if (strncmp(token, "Name", sizeof("Name")) == 0) {
      proc_info_struct->Name = strdup(saveptr);
    }

    if (strncmp(token, "Umask", sizeof("Umask")) == 0) {
      proc_info_struct->Umask = atoi(saveptr);
    }

    if (strncmp(token, "State", sizeof("State")) == 0) {

      token = strtok_r(NULL, " ", &saveptr);
      proc_info_struct->Status = strdup(saveptr);
    }

    if (strncmp(token, "Uid", sizeof("Uid")) == 0) {

      token = strtok_r(NULL, " ", &saveptr);
      proc_info_struct->user_name = strdup(get_user(atoi(token)));
    }
    i++;
  }
  return proc_info_struct;
}

size_t writer_log(const int log_fd, const char *watched_path,
                  proc_info_t *proc_info_struct) {
  // tokenize buf and structure it for writing to the log file.
  // - process name and uid for the user.
  // -file path as well

  return EXIT_SUCCESS;
}
