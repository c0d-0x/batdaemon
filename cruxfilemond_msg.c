#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOCK_FILE "cf.lock"
void help(char *prog) {
  fprintf(stderr, "usage:%s < -SIGNAL >\n", prog);
  printf(" SIGKILL: %d\n SIGTERM: %d\n SIGINT: %d\n SIGHUP: %d\n SIGUSR1: %d\n",
         SIGKILL, SIGTERM, SIGINT, SIGHUP, SIGUSR1);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    help(argv[0]);
    return EXIT_FAILURE;
  }

  char buf[64] = {0x0};
  int fd_log;
  int pid_cruxfilemond;
  int sig = 0;
  fd_log = open(LOCK_FILE, O_RDONLY);
  if (fd_log == -1) {
    perror("Fail to open LOCK_FILE");
    return EXIT_FAILURE;
  }

  read(fd_log, buf, sizeof(int64_t));
  close(fd_log);

  pid_cruxfilemond = atoi(buf);
  sig = atoi(argv[1]);
  if (sig >= 0 || pid_cruxfilemond < 2) {
    help(argv[0]);
    return EXIT_FAILURE;
  }
  if (kill(pid_cruxfilemond, -sig) != 0) {
    fprintf(stderr, "Failed to send signal: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  fprintf(stdout, "signal %d sent to PID: %d!!\n", -sig, pid_cruxfilemond);
  return EXIT_SUCCESS;
}
