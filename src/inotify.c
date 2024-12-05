#include "inotify.h"

#include <signal.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "core.h"
#include "debug.h"
#include "main.h"

int init_inotify(char *file_path) {
  int inotify_fd;
  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd == -1) {
    DEBUG("Failed to initialize inotify");
    return CUSTOM_ERR;
  }

  if (inotify_add_watch(inotify_fd, file_path, IN_MODIFY | IN_CREATE) == -1) {
    DEBUG("Add Watch Failure: %s ", strerror(errno));
    return CUSTOM_ERR;
  }
  return inotify_fd;
}

config_t *inotify_event_handler(int inotify_fd, int config_fd,
                                config_t *(*handler)(int config_fd)) {
  int len;
  config_t *config_obj = NULL;
  struct inotify_event *event;
  char buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

  while ((len = read(inotify_fd, buffer, sizeof(buffer))) > 0) {
    for (char *buf_prt = buffer; buf_prt < buffer + len;
         buf_prt += sizeof(struct inotify_event) + event->len) {
      event = (struct inotify_event *)buf_prt;

      if ((event->mask & IN_MODIFY) || (event->mask & IN_CREATE)) {
        DEBUG("%s is modified", event->name);
        config_obj = handler(config_fd);

        if (inotify_add_watch(inotify_fd, CF_HOME_DIR, IN_MODIFY | IN_CREATE) ==
            -1) {
          DEBUG("Add Watch Failure: %s ", strerror(errno));
          kill(getpid(), SIGTERM);
        }

        return config_obj;
      }
    }
  }

  if (len == -1 && errno != EAGAIN) {
    DEBUG("read syscall Failed: %s", strerror(errno));
    kill(getpid(), SIGTERM);
  }
  return NULL;
}
