#include "daemon.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage batterymond <option> <file path>\n");
    return EXIT_FAILURE;
  }

  char *file_name;
  char *file_path = strdup(argv[2]);

  if (strncmp(argv[1], "-p", 2) == 0) {
    file_name = load_file(file_path);
  }

  int file_dscripto = -1, watch_ptr = -1;

  file_dscripto = inotify_init();
  if (file_dscripto == -1) {
    perror("File Handler Failure");
    return EXIT_FAILURE;
  }

  watch_ptr = inotify_add_watch(file_dscripto, argv[2], IN_MODIFY);
  if (watch_ptr == -1) {
    perror("Add Watch Failure");
    return EXIT_FAILURE;
  }

  struct inotify_event *watch_event;
  int len;
  char buffer[4096];

  while (true) {
    len = read(file_dscripto, buffer, sizeof(buffer));
    if (len == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    for (char *buf_prt = buffer; buf_prt < buffer + len;
         buf_prt += sizeof(struct inotify_event) + watch_event->len) {

      watch_event = (struct inotify_event *)buf_prt;
      if (watch_event->mask & IN_MODIFY) {
        fprintf(stdout, "%s is modified\n", file_name);
        break;
      }
    }
  }

  return EXIT_SUCCESS;
}
