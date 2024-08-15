#ifndef DAEMONZ_H
#define DAEMONZ_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

void _daemonize(void);

#endif /* ifndef DAEMONZ_H */
