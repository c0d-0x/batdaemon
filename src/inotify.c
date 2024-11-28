#include "inotify.h"

#include <string.h>
#include <sys/inotify.h>

#include "core.h"
#include "debug.h"

int init_inotify(char *file_path) {
  int inotify_fd;
  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd == -1) {
    DEBUG("Failed to initialize inotify");
    return CUSTOM_ERR;
  }

  if (inotify_add_watch(inotify_fd, file_path, IN_MODIFY | IN_DELETE) == -1) {
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
  char buffer[4096] = {0x0};

  while (1) {
    len = read(inotify_fd, buffer, sizeof(buffer));
    if (len == -1 && errno != EAGAIN) {
      DEBUG("read syscall Failed: %s", strerror(errno));
      kill(getpid(), SIGTERM);
    } else if (errno == EAGAIN)
      continue;

    if (len == 0) return NULL;
    for (char *buf_prt = buffer; buf_prt < buffer + len;
         buf_prt += sizeof(struct inotify_event) + event->len) {
      event = (struct inotify_event *)buf_prt;
      if (event->mask & IN_MODIFY) {
        DEBUG("%s is modified", event->name);
        config_obj = handler(config_fd);
        return config_obj;
      }
    }
  }
  return NULL;
}
