#include "filemond.h"
#include "logger.h"
#include <err.h>
#include <fcntl.h>
#include <linux/fanotify.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

config_t *load_config_file(char *file_Path) {
  struct stat path_stat;
  size_t i = 0, index_n = -1, F_Flag;
  char buffer[PATH_MAX], *tok;
  config_t *config_obj;
  FILE *fp_config = NULL;

  if (access(file_Path, F_OK) != 0) {
    fprintf(stderr, "Config file not found!!\n");
    exit(EXIT_FAILURE);
  }

  if ((fp_config = fopen(file_Path, "r")) == NULL) {
    perror("Fail to open Config file\n");
    return NULL;
  }

  config_obj = malloc(sizeof(config_t));
  while (fgets(buffer, sizeof(buffer), fp_config) != NULL && errno != EOF &&
         i < MAX_WATCH) {

    if ((tok = strchr(buffer, '\n')) != NULL) {
      index_n = tok - buffer;
      buffer[index_n] = '\0';
    }

    if (stat(buffer, &path_stat) == 0) {
      if (path_stat.st_mode & S_IFDIR) {
        F_Flag = F_IS_DIR;
      } else if (path_stat.st_mode & S_IFREG) {
        F_Flag = F_IS_FILE;
      } else {
        continue;
      }
    } else {
      fclose(fp_config);
      fprintf(stderr, "%s\n -> Invalid input from the config\n", buffer);
      exit(EXIT_FAILURE);
    }

    config_obj->watchlist_len = i + 1;
    config_obj->watchlist[i].F_TYPE = F_Flag;
    (config_obj->watchlist[i].path) = strdup(buffer);
    i++;
  }
  fclose(fp_config);
  return config_obj;
}

void config_obj_cleanup(config_t *config_obj) {
  for (size_t s = 0; s < config_obj->watchlist_len; s++) {
    free(config_obj->watchlist[s].path);
  }
  free(config_obj);
}

size_t check_lock(char *path_lock) {
  if (access(path_lock, F_OK) != 0) {
    pid_t pid = getpid();
    FILE *fp_lock = NULL;
    if ((fp_lock = fopen(path_lock, "w")) == NULL) {
      perror("Could not create lock file");
      return CUSTOM_ERR;
    }
    fprintf(fp_lock, "%d", pid);
    fclose(fp_lock);
    return EXIT_SUCCESS;
  }

  fprintf(stderr,
          "An instance of cruxfilemond is already running\n"
          "If no cruxfilemond instance is running, Delete '%s' file \n",
          LOCK_FILE);
  return CUSTOM_ERR;
}

void fan_event_handler(int fan_fd) {
  const struct fanotify_event_metadata *metadata;
  struct fanotify_event_metadata buf[200] = {0x0};
  char *buffer[11] = {0x0};
  ssize_t len;
  char path[PATH_MAX] = {0x0};
  proc_info_t *procinfo;
  ssize_t path_len, p_event;
  char procfd_path[PATH_MAX] = {0x0};
  struct fanotify_response response;

  while (true) {

    /* Read some events. */

    len = read(fan_fd, buf, sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /* Check if end of available data reached. */

    if (len <= 0)
      break;

    /* Point to the first event in the buffer. */

    metadata = buf;

    /* Loop over all events in the buffer. */

    while (FAN_EVENT_OK(metadata, len)) {

      /* Check that run-time and compile-time structures match. */

      if (metadata->vers != FANOTIFY_METADATA_VERSION) {
        fprintf(stderr, "Mismatch of fanotify metadata version.\n");
        exit(EXIT_FAILURE);
      }

      /* metadata->fd contains either FAN_NOFD, indicating a
         queue overflow, or a file descriptor (a nonnegative
         integer). Here, queue overflow is simply ignored. */

      if (metadata->fd >= 0) {
        /* Handle open permission event. */
        if (metadata->mask & FAN_OPEN_PERM) {
          p_event = FAN_OPEN_PERM;
          proc_info(metadata->pid, buffer, 11);

          /* Allow file to be opened. */

          response.fd = metadata->fd;
          response.response = FAN_ALLOW;
          write(fan_fd, &response, sizeof(response));
        } else if (metadata->mask & FAN_MODIFY) {
          p_event = FAN_MODIFY;
          proc_info(metadata->pid, buffer, 11);
        }

        /* Retrieve and print pathname of the accessed file. */

        snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d",
                 metadata->fd);
        path_len = readlink(procfd_path, path, sizeof(path) - 1);
        if (path_len == -1) {
          perror("readlink");
          exit(EXIT_FAILURE);
        }

        path[path_len] = '\0';

        if ((procinfo = load_proc_info(buffer)) == NULL) {
          fprintf(stderr, "Fail to load effective process's info\n");
        }
        procinfo->file_path = path;
        procinfo->p_event =
            (p_event == FAN_MODIFY) ? "FILE MODIFIED" : "FILE ACCESSED";

        writer_log(stdout, procinfo);
        cleanup_procinfo(procinfo);
        close(metadata->fd);
      }

      /* Advance to next event. */
      metadata = FAN_EVENT_NEXT(metadata, len);
    }
  }
}
