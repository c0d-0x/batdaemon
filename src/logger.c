#include "logger.h"
#include "filemond.h"

void proc_info(pid_t pid, char *buffer[], size_t buf_max) {
  char procfd_path[32] = {0x0};
  char buf_temp[64] = {0x0};
  char *tok = NULL;
  FILE *proc_fd = NULL;
  size_t index_n, i = 0;

  snprintf(procfd_path, sizeof(procfd_path), "/proc/%d/status", pid);

  procfd_path[strlen(procfd_path)] = '\0';
  if (access(procfd_path, F_OK) == 0) {
    printf("F: %s exist\n", procfd_path);
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

size_t get_user_group(const uid_t uid, const gid_t gid,
                      proc_info_t *proc_info_struct) {
  struct passwd *pws;
  struct group *grp;
  if ((grp = getgrgid(gid)) != NULL)
    proc_info_struct->group_name = strdup(grp->gr_name);

  if ((pws = getpwuid(uid)) != NULL)
    proc_info_struct->user_name = strdup(pws->pw_name);

  if (proc_info_struct->group_name != NULL &&
      proc_info_struct->user_name != NULL)
    return EXIT_SUCCESS;
  return CUSTOM_ERR;
}

size_t writer_log(const int log_fd, const char *watched_path,
                  proc_info_t *proc_info_struct) {
  // tokenize buf and structure it for writing to the log file.
  // - process name and uid for the user.
  // -file path as well

  return EXIT_SUCCESS;
}
