#ifndef DAEMONZ_H
#define DAEMONZ_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

static pid_t pid;
#define DAEMONIZE(void)                 \
  pid = fork();                         \
  if (pid < 0) exit(EXIT_FAILURE);      \
  if (pid > 0) exit(EXIT_SUCCESS);      \
  if (setsid() < 0) exit(EXIT_FAILURE); \
  pid = fork();                         \
  if (pid < 0) exit(EXIT_FAILURE);      \
  if (pid > 0) exit(EXIT_SUCCESS);      \
  umask(0);                             \
  openlog("cruxfilemond", LOG_PID, LOG_DAEMON);

#endif /* ifndef DAEMONZ_H */
