#define _GNU_SOURCE
#include "./src/main.h"

#include <fcntl.h>
#include <unistd.h>

#include "./src/daemonz.h"
#include "./src/debug.h"
#include "./src/filemond.h"
#include "src/inotify.h"

int config_fd;
size_t debug = 0;
FILE* fp_log = NULL;
int fan_fd, inotify_fd;
config_t* config_obj = NULL;

void help(char* argv);
void signal_handler(int sig);
static void parse_options(const int argc, char* argv[]);
static void fan_mark_wraper(int fd, config_t* config_obj);

int main(int argc, char* argv[]) {
  nfds_t nfds;
  int poll_num;
  FILE* fp_lock;
  struct pollfd fds[2];
  struct sigaction sigact;

  parse_options(argc, argv);
  if (check_lock(LOCK_FILE) != 0) exit(EXIT_FAILURE);
  if (!debug) _daemonize();

  if ((fp_lock = fopen(LOCK_FILE, "w")) == NULL) {
    DEBUG("Failed to open %s file: %s", LOG_FILE, strerror(errno));
    exit(EXIT_FAILURE);
  }

  fprintf(fp_lock, "%d", getpid());
  fclose(fp_lock);
  DEBUG("cruxfilemond Started");

  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = signal_handler;
  sigact.sa_flags = SA_RESTART;

  DEBUG("Making receptions for signals");
  if (sigaction(SIGTERM, &sigact, NULL) != 0 ||
      sigaction(SIGINT, &sigact, NULL) != 0) {
    DEBUG("Fail to make reception for signals");
    exit(EXIT_FAILURE);
  }

  config_fd = open(CONFIG_FILE, O_RDONLY | O_NONBLOCK);
  if (config_fd == -1) {
    DEBUG("Failed to open the config file: %s", CONFIG_FILE);
    kill(getpid(), SIGTERM);
  }

  if ((fp_log = fopen(LOG_FILE, "w+")) == NULL) {
    DEBUG("Failed to open LOG_FILE: %s", strerror(errno));
    kill(getpid(), SIGTERM);
  }

  DEBUG("LOG_FILE opened: %s", LOG_FILE);
  DEBUG("Initializing an Fa_Notify instance");
  // fanotify for mornitoring files.
  fan_fd = fanotify_init(FAN_CLOEXEC | FAN_NONBLOCK, O_RDONLY | O_LARGEFILE);
  if (fan_fd == -1) {
    DEBUG("Failed to initializing an Fa_Notify instance: %s", strerror(errno));
    kill(getpid(), SIGTERM);
  }

  /*Watch the config file for changes*/
  inotify_fd = init_inotify(CONFIG_FILE);
  if (inotify_fd == -1) {
    kill(getpid(), SIGTERM);
  }

  DEBUG("A valid Fa_Notify file descriptor: initialized");
  config_obj = parse_config_file(config_fd);
  if (config_obj == NULL || config_obj->watchlist_len == 0 ||
      config_obj->watchlist->path == NULL) {
    DEBUG("%s: Error! Add valid files and dirs to be watched", CONFIG_FILE);
    kill(getpid(), SIGTERM);
  }

  DEBUG("Marking watchlist...");
  fan_mark_wraper(fan_fd, config_obj); /* Adds watched items to fan_fd*/
  config_obj_cleanup(config_obj);
  /*pause();*/
  nfds = 2;
  fds[0].fd = fan_fd; /* Fanotify input */
  fds[0].events = POLLIN;

  fds[1].fd = inotify_fd; /* inotify input */
  fds[1].events = POLLIN;

  DEBUG("Setting up a Poll instance for the watchlist events");
  while (true) {
    poll_num = poll(fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR) /* Interrupted by a signal */
        continue;         /* Restart poll() */

      DEBUG("Poll Failed: %s", strerror(errno));
      kill(getpid(), SIGTERM);
    }

    if (poll_num > 0) {
      if (fds[0].revents & POLLIN) fan_event_handler(fan_fd, fp_log);

      if (fds[1].revents & POLLIN) {
        if ((config_obj = inotify_event_handler(inotify_fd, config_fd,
                                                parse_config_file)) == NULL)
          continue;
        DEBUG(
            "CONFIG_FILE :%s edited\nFlushing the watchlist from the "
            "fanotify_markfd",
            CONFIG_FILE);
        if (fanotify_mark(fan_fd, FAN_MARK_FLUSH,
                          FAN_OPEN | FAN_MODIFY | FAN_EVENT_ON_CHILD, AT_FDCWD,
                          NULL) == -1) {
          DEBUG("Fanotify_Mark: Failed!!!");
          kill(getpid(), SIGTERM);
        }

        fan_mark_wraper(fan_fd, config_obj);
        config_obj_cleanup(config_obj);
      }
    }
  }
}

static void fan_mark_wraper(int fd, config_t* config_obj) {
  size_t i = 0;
  while (i < config_obj->watchlist_len) {
    if (fanotify_mark(fd,
                      (config_obj->watchlist[i].F_TYPE)
                          ? FAN_MARK_ADD | FAN_MARK_ONLYDIR
                          : FAN_MARK_ADD,
                      FAN_OPEN | FAN_MODIFY | FAN_EVENT_ON_CHILD, AT_FDCWD,
                      config_obj->watchlist[i].path) == -1) {
      DEBUG("Fanotify_Mark: Failed to mark files from config");
      kill(getpid(), SIGTERM);
    }
    DEBUG("%s: Marked", config_obj->watchlist[i].path);
    i++;
  }
}

void signal_handler(int sig) {
  if (sig == SIGTERM || sig == SIGINT) {
    // necessary clean up then exit
    if (fp_log != NULL) fclose(fp_log);
    remove(LOCK_FILE);
    close(config_fd);
    DEBUG("Terminating cruxfilemond");
    closelog();
    exit(EXIT_SUCCESS);
  }
}

void help(char* argv) {
  fprintf(stdout, "%s < -option >", argv);
  fprintf(stdout,
          "options\n -d: debug mode will prevent cruxfilemond from as "
          "a daemon process\n");
}

static void parse_options(const int argc, char* argv[]) {
  if (getuid() != 0) {
    fprintf(stderr, "Run %s as root!!\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (argc > 1) {
    if (strncmp("-d", argv[1], 3) == 0) {
      debug = 1;
    } else {
      help(argv[0]);
      exit(EXIT_SUCCESS);
    }
  }
}
