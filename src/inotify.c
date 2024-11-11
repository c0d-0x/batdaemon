#include "inotify.h"

#include <string.h>
#include <sys/inotify.h>

#include "debug.h"
#include "filemond.h"

int init_inotify(char *file_path) {
  int inotify_fd;
  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd == -1) {
    DEBUG("Failed to initialize inotify\n", NULL);
    return (-1);
  }

  if (inotify_add_watch(inotify_fd, file_path, IN_MODIFY | IN_NONBLOCK) == -1) {
    DEBUG("Add Watch Failure: ", strerror(errno));
    return (-1);
  }
  return inotify_fd;
}

config_t *inotify_event_handler(int inotify_fd, int config_fd,
                                config_t *(*handler)(int config_fd)) {
  struct inotify_event *event;
  int len;
  config_t *config_obj = NULL;
  char buffer[200] = {0x0};

  while (1) {
    len = read(inotify_fd, buffer, sizeof(buffer));
    if (len == -1 && errno != EAGAIN) {
      perror("read syscall Failed");
      exit(EXIT_FAILURE);
    } else if (errno == EAGAIN)
      break;

    if (len == 0) return NULL;
    for (char *buf_prt = buffer; buf_prt < buffer + len;
         buf_prt += sizeof(struct inotify_event) + event->len) {
      event = (struct inotify_event *)buf_prt;
      if (event->mask & IN_MODIFY) {
        DEBUG(event->name, "is modified\n");
        config_obj = handler(config_fd);
        return config_obj;
      }
    }
  }
  return NULL;
}
