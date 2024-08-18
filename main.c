#define _GNU_SOURCE
#include "./src/daemonz.h"
#include "./src/filemond.h"
#include "./src/logger.h"
#include <sys/syslog.h>

config_t *config_obj = NULL;
char *buffer = NULL;
FILE *fp_log = NULL, *fp_tmp_log = NULL;
int fan_fd;

void signal_handler(int sig);
static void fan_mark_wraper(int fd, config_t *config_obj);

int main(int argc, char *argv[]) {

  _daemonize();
  syslog(LOG_NOTICE, "cruxfilemond Started");
  int poll_num;
  nfds_t nfds;
  struct pollfd fds;
  if (check_lock(LOCK_FILE) != 0) {
    exit(EXIT_FAILURE);
  }

  struct sigaction sigact;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = signal_handler;
  sigact.sa_flags = SA_RESTART;
  if (sigaction(SIGHUP, &sigact, NULL) != 0 ||
      sigaction(SIGTERM, &sigact, NULL) != 0 ||
      sigaction(SIGUSR1, &sigact, NULL) != 0 ||
      sigaction(SIGINT, &sigact, NULL) != 0) {
    syslog(LOG_ERR, "Fail to make reception for signals\n");
    exit(EXIT_FAILURE);
  }

  if ((fp_log = fopen(LOG_FILE, "a+")) == NULL) {
    syslog(LOG_ERR, "Fail to open logfile: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // fanotify for mornitoring files.
  fan_fd = fanotify_init(FAN_CLOEXEC | FAN_NONBLOCK, O_RDONLY | O_LARGEFILE);
  if (fan_fd == -1) {
    syslog(LOG_ERR, "Fanotify_Init Failed to initialize");
    exit(EXIT_FAILURE);
  }

  config_obj = load_config_file(CONFIG_FILE);
  if (config_obj->watchlist_len == 0 || config_obj->watchlist->path == NULL) {
    syslog(LOG_ERR, "%s is empty! Add files or dirs to be watched",
           CONFIG_FILE);
    exit(EXIT_FAILURE);
  }

  fan_mark_wraper(fan_fd, config_obj); /* Adds watched items to fan_fd*/
  config_obj_cleanup(config_obj);
  nfds = 1;
  fds.fd = fan_fd; /* Fanotify input */
  fds.events = POLLIN;

  while (true) {

    poll_num = poll(&fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR) /* Interrupted by a signal */
        continue;         /* Restart poll() */

      syslog(LOG_ERR, "Poll Failed"); /* Unexpected error */
      exit(EXIT_FAILURE);
    }

    if (poll_num > 0) {
      if (fds.revents & POLLIN)
        fan_event_handler(fan_fd, fp_log);
    }
  }
}

static void fan_mark_wraper(int fd, config_t *config_obj) {

  size_t i = 0;
  while (i < config_obj->watchlist_len) {

    if (fanotify_mark(fd,
                      (config_obj->watchlist[i].F_TYPE)
                          ? FAN_MARK_ADD | FAN_MARK_ONLYDIR
                          : FAN_MARK_ADD,
                      FAN_OPEN | FAN_MODIFY | FAN_EVENT_ON_CHILD, AT_FDCWD,
                      config_obj->watchlist[i].path) == -1) {
      syslog(LOG_ERR, "Fanotify_Mark: Failed to mark files from config");
      exit(EXIT_FAILURE);
    }
    // for debugging and testing purposes
    // printf("[%ld]-Path: %s - %ld \n", i, (config_obj->watchlist[i].path),
    //        (config_obj->watchlist[i].F_TYPE));
    i++;
  }
}

void signal_handler(int sig) {
  switch (sig) {

  case SIGHUP:

    config_obj = load_config_file(CONFIG_FILE);
    if (fanotify_mark(fan_fd, FAN_MARK_FLUSH,
                      FAN_OPEN | FAN_MODIFY | FAN_EVENT_ON_CHILD, AT_FDCWD,
                      NULL) == -1) {
      syslog(LOG_ERR, "Fanotify_Mark");
      exit(EXIT_FAILURE);
    }

    fan_mark_wraper(fan_fd, config_obj);
    config_obj_cleanup(config_obj);
    return;

  case SIGUSR1:

    if ((fp_tmp_log = fopen(LOG_FILE, "r")) != NULL) {
      if ((buffer = calloc(256, sizeof(char))) == NULL) {
        fclose(fp_tmp_log);
        perror("Fail to allocate memory");
        exit(EXIT_FAILURE);
      }

      while (fgets(buffer, sizeof(buffer), fp_tmp_log) != NULL) {
        fprintf(stdout, "%s", buffer);
      }

      fclose(fp_tmp_log);
      free(buffer);
    } else {
      fprintf(stderr, "No File have been modified\n");
    }
    return;
  }

  if (sig == SIGTERM || sig == SIGINT) {

    // necessary clean up then exit
    if (fp_log != NULL)
      fclose(fp_log);
    remove(LOCK_FILE);
    syslog(LOG_NOTICE, "cruxfilemond terminated");
    closelog();
    exit(EXIT_SUCCESS);
  }
}
