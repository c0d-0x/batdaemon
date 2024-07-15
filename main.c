#include "./src/filemond.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

config_t *config_obj;
char buffer[256];
FILE *fp_log = NULL;

void signal_handler(int sig);

int main(int argc, char *argv[]) {

  size_t i = 0;
  struct sigaction sigact;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = signal_handler;
  sigact.sa_flags = SA_RESTART;
  if (sigaction(SIGHUP, &sigact, NULL) != 0 ||
      sigaction(SIGTERM, &sigact, NULL) != 0 ||
      sigaction(SIGUSR1, &sigact, NULL) != 0 ||
      sigaction(SIGINT, &sigact, NULL) != 0) {
    fprintf(stderr, "Fall to make reception for signals\n");
    exit(EXIT_FAILURE);
  }

  if ((config_obj = load_config_file(CONFIG_FILE)) == NULL) {
    exit(EXIT_FAILURE);
  }

  if (check_lock(LOCK_FILE) != 0) {
    exit(EXIT_FAILURE);
  }

  if ((fp_log = fopen(LOCK_FILE, "a")) == NULL) {
    perror("Fail to open logfile");
    exit(EXIT_FAILURE);
  }

  while (i <= config_obj->watchlist_len) {
    printf("[%ld]-Path: %s\n", i, (config_obj->watchlist[i]));
    i++;
  }

  config_obj_cleanup(config_obj);
  while (1)
    ;
}

void signal_handler(int sig) {
  switch (sig) {

  case SIGHUP:
    config_obj = load_config_file(CONFIG_FILE);
    //[TODO]: Add to the watch list
    //[TODO]: Modify load_config_file() func to filter the watchlist.
    printf("\n From SIGHUP\n");
    for (size_t i = 0; i <= config_obj->watchlist_len; i++) {
      printf("[%ld]-Path: %s\n", i, (config_obj->watchlist[i]));
    }
    config_obj_cleanup(config_obj);
    return;

  case SIGUSR1:
    if (fp_log)
      fclose(fp_log);

    if ((fp_log = fopen(LOG_FILE, "r")) != NULL) {
      while (fread(buffer, 1, sizeof(buffer), fp_log) != 0) {
        fprintf(stdin, "%s", buffer);
        memset(buffer, '\0', strlen(buffer));
      }

      fp_log = fopen(LOG_FILE, "a");
    } else {
      fprintf(stderr, "No File have been modified\n");
    }
    return;
  }

  if (sig == SIGTERM || sig == SIGINT) {
    // necessary clean up then exit
    remove(LOCK_FILE);
    exit(EXIT_SUCCESS);
  }
}
