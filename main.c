#define _GNU_SOURCE
#include "./src/daemonz.h"
#include "./src/debug.h"
#include "./src/filemond.h"
#include "./src/logger.h"

int fan_fd;
size_t debug = 0;
char *buffer = NULL;
FILE *fp_log = NULL;
config_t *config_obj = NULL;

void help(char *argv);
void signal_handler(int sig);
static void fan_mark_wraper(int fd, config_t *config_obj);

int main(int argc, char *argv[]) {

  if (argc > 1) {
    if (strncmp("-d", argv[1], 3) == 0) {
      debug = 1;
    } else {
      help(argv[0]);
      exit(EXIT_SUCCESS);
    }
  }

  FILE *fp_lock;
  if (getuid() != 0) {
    fprintf(stderr, "Run %s as root!!\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (check_lock(LOCK_FILE) != 0) {
    exit(EXIT_FAILURE);
  }

  if (!debug)
    _daemonize();

  /* Open the syslog file */
  openlog("cruxfilemond", LOG_PID, LOG_DAEMON);
  if ((fp_lock = fopen(LOCK_FILE, "w")) == NULL) {
    syslog(LOG_INFO, "Failed to open %s file: %s", LOCK_FILE, strerror(errno));
    DEBUG("Failed to open %s file: ", strerror(errno));
    exit(EXIT_FAILURE);
  }

  fprintf(fp_lock, "%d", getpid());
  fclose(fp_lock);
  syslog(LOG_NOTICE, "cruxfilemond Started");
  DEBUG("cruxfilemond Started\n", NULL);
  int poll_num;
  nfds_t nfds;
  struct pollfd fds;

  struct sigaction sigact;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = signal_handler;
  sigact.sa_flags = SA_RESTART;

  DEBUG("Making receptions for signals\n", NULL);
  if (sigaction(SIGHUP, &sigact, NULL) != 0 ||
      sigaction(SIGTERM, &sigact, NULL) != 0 ||
      sigaction(SIGINT, &sigact, NULL) != 0) {
    syslog(LOG_ERR, "Fail to make reception for signals\n");
    DEBUG("Fail to make reception for signals\n", NULL);
    exit(EXIT_FAILURE);
  }

  if ((fp_log = fopen(LOG_FILE, "a+")) == NULL) {
    syslog(LOG_ERR, "Fail to open logfile: %s", strerror(errno));
    DEBUG("Failed to open LOG_FILE\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  DEBUG("LOG_FILE opened: ", LOG_FILE);
  DEBUG("Initializing an Fa_Notify instance\n", NULL);
  // fanotify for mornitoring files.
  fan_fd = fanotify_init(FAN_CLOEXEC | FAN_NONBLOCK, O_RDONLY | O_LARGEFILE);
  if (fan_fd == -1) {
    syslog(LOG_ERR, "Fanotify_Init Failed to initialize");
    DEBUG("Failed to initializing an Fa_Notify instance", strerror(errno));
    exit(EXIT_FAILURE);
  }

  DEBUG("A valid Fa_Notify file descriptor: initialized\n", NULL);
  config_obj = load_config_file(CONFIG_FILE);
  if (config_obj->watchlist_len == 0 || config_obj->watchlist->path == NULL) {
    syslog(LOG_ERR, "%s is empty! Add files or dirs to be watched",
           CONFIG_FILE);
    DEBUG(CONFIG_FILE, "is empty! Add files or dirs to be watched\n");
    exit(EXIT_FAILURE);
  }

  DEBUG("Marking watchlist for mornitoring\n", NULL);
  fan_mark_wraper(fan_fd, config_obj); /* Adds watched items to fan_fd*/

  config_obj_cleanup(config_obj);
  nfds = 1;
  fds.fd = fan_fd; /* Fanotify input */
  fds.events = POLLIN;

  DEBUG("Setting up a Poll instance for the watchlist events\n", NULL);
  while (true) {

    poll_num = poll(&fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR) /* Interrupted by a signal */
        continue;         /* Restart poll() */

      syslog(LOG_ERR, "Poll Failed"); /* Unexpected error */
      DEBUG("Poll Failed\n", NULL);
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
      DEBUG("Fanotify_Mark: Failed to mark files from config\n", NULL);

      exit(EXIT_FAILURE);
    }
    DEBUG(config_obj->watchlist[i].path, ":Successfully added");
    i++;
  }
}

void signal_handler(int sig) {

  if (sig == SIGHUP) {

    config_obj = load_config_file(CONFIG_FILE);
    DEBUG(
        "SIGHUP: Received\nFlushing the watchlist from the fanotify_mark fd\n",
        NULL);
    if (fanotify_mark(fan_fd, FAN_MARK_FLUSH,
                      FAN_OPEN | FAN_MODIFY | FAN_EVENT_ON_CHILD, AT_FDCWD,
                      NULL) == -1) {
      syslog(LOG_ERR, "Fanotify_Mark");
      exit(EXIT_FAILURE);
    }

    fan_mark_wraper(fan_fd, config_obj);
    config_obj_cleanup(config_obj);
  }

  if (sig == SIGTERM || sig == SIGINT) {

    // necessary clean up then exit
    if (fp_log != NULL)
      fclose(fp_log);
    remove(LOCK_FILE);
    DEBUG("Terminating cruxfilemond\n", NULL);
    syslog(LOG_NOTICE, "cruxfilemond terminated");
    closelog();
    exit(EXIT_SUCCESS);
  }
}

void help(char *argv) {
  fprintf(stdout, "%s < -option >", argv);
  fprintf(stdout, "options\n -d: debug mode will prevent cruxfilemond from as "
                  "a daemon process\n");
}
