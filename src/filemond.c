#include "filemond.h"

#include "config.h"
#include "debug.h"
#include "logger.h"
#include "notify.h"
// NotifyNotification *notify_instance;
config_t *load_config_file(char *file_Path) {
  DEBUG("Loading watchlist from the CONFIG_FILE: ", CONFIG_FILE);
  struct stat path_stat;
  size_t i = 0, index_n = -1, F_Flag;
  char buffer[PATH_MAX], *tok;
  config_t *config_obj;
  FILE *fp_config = NULL;

  if (access(file_Path, F_OK) != 0) {
    syslog(LOG_ERR, "Config file not found!!\n");
    DEBUG("Config file not found!!\n", NULL);
    exit(EXIT_FAILURE);
  }

  if ((fp_config = fopen(file_Path, "r")) == NULL) {
    syslog(LOG_ERR, "Fail to open Config file\n");
    DEBUG("Fail to open Config file: ", strerror(errno));
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
      syslog(LOG_ERR, "%s\n -> Invalid input from the config\n", buffer);
      DEBUG(buffer, "->Invalid input from the config");
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
  DEBUG("watchlist clean up\n", NULL);
  for (size_t i = 0; i < config_obj->watchlist_len; i++) {
    free(config_obj->watchlist[i].path);
  }
  free(config_obj);
}

size_t check_lock(char *path_lock) {
  if (access(path_lock, F_OK) != 0) {
    DEBUG("No LOCK_FILE found: No instance of cruxfilemond running\n", NULL);
    FILE *fp_lock = NULL;
    if ((fp_lock = fopen(path_lock, "w")) == NULL) {
      perror("Could not create lock file");
      DEBUG("Could not create lock file: ", strerror(errno));
      return CUSTOM_ERR;
    }
    return EXIT_SUCCESS;
  }

  fprintf(stderr,
          "An instance of cruxfilemond is already running\n"
          "If no cruxfilemond instance is running, Delete '%s' file \n",
          LOCK_FILE);
  return CUSTOM_ERR;
}

void fan_event_handler(int fan_fd, FILE *fp_log) {
  const struct fanotify_event_metadata *metadata;
  struct fanotify_event_metadata buf[200] = {0x0};
  char *buffer[11] = {0x0};
  ssize_t len;
  cus_stack_t *__stack = NULL;
  cus_stack_t *__stack_ptr = NULL;
  char path[PATH_MAX] = {0x0};
  proc_info_t *procinfo;
  ssize_t path_len, p_event;
  char procfd_path[PATH_MAX] = {0x0};
  // struct fanotify_response response;

  while (true) {
    /* Read some events. */
    len = read(fan_fd, buf, sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      syslog(LOG_ERR, "Failed to read fan_events");
      DEBUG("Failed to read fan_events", NULL);
      exit(EXIT_FAILURE);
    }

    /* Check if end of available data reached. */
    if (len <= 0) break;

    /* Point to the first event in the buffer. */

    metadata = buf;

    /* Loop over all events in the buffer. */

    while (FAN_EVENT_OK(metadata, len)) {
      /* Check that run-time and compile-time structures match. */

      if (metadata->vers != FANOTIFY_METADATA_VERSION) {
        syslog(LOG_ERR, "Mismatch of fanotify metadata version.\n");
        DEBUG("Mismatch of fanotify metadata version\n", NULL);
        exit(EXIT_FAILURE);
      }

      /* metadata->fd contains either FAN_NOFD, indicating a
         queue overflow, or a file descriptor (a nonnegative
         integer). Here, queue overflow is simply ignored. */

      if (metadata->fd >= 0) {
        /* Handle open permission event. */
        if (metadata->mask & FAN_OPEN) {
          p_event = FAN_OPEN;
          proc_info(metadata->pid, buffer, 11);

        } else if (metadata->mask & FAN_MODIFY) {
          p_event = FAN_MODIFY;
          proc_info(metadata->pid, buffer, 11);
        }

        /* Retrieve and print pathname of the accessed file. */

        snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d",
                 metadata->fd);
        path_len = readlink(procfd_path, path, sizeof(path) - 1);
        if (path_len == -1) {
          syslog(LOG_ERR, "readlink: %s", strerror(errno));
          DEBUG("readlink: ", strerror(errno));
          exit(EXIT_FAILURE);
        }

        path[path_len] = '\0';

        if ((procinfo = load_proc_info(buffer)) == NULL) {
          syslog(LOG_ERR, "Fail to load effective process's info");
          DEBUG("Failed to load effective process's info\n", NULL);
        }
        procinfo->file_path = path;
        procinfo->p_event =
            (p_event == FAN_MODIFY) ? "FILE MODIFIED" : "FILE ACCESSED";

        DEBUG("Event registered:", procinfo->p_event);
        DEBUG(procinfo->file_path, "\n");

        push_stk(&__stack, procinfo);
        close(metadata->fd);
      }
      /* Advance to next event. */
      metadata = FAN_EVENT_NEXT(metadata, len);
    }

    while (__stack != NULL) {
      __stack_ptr = pop_stk(&__stack);
      writer_log(fp_log, __stack_ptr->data);
      //[TODO]: send a notification to the system notification daemon
      notify_send_msg(__stack_ptr->data, NOTIFY_URGENCY_NORMAL);
      cleanup_procinfo(__stack_ptr->data);
      free(__stack_ptr);
    }
  }
  close_notification();
}
