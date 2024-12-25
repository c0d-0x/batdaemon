
#include <sys/syslog.h>
#ifndef DEBUG_H
#include <stdio.h>
extern size_t debug;

#define DEBUG(fmt, ...)                                         \
  do {                                                          \
    if (debug) {                                                \
      fprintf(stderr, "[cruxfilemond]\x20" fmt, ##__VA_ARGS__); \
      fprintf(stderr, "\n");                                    \
    } else {                                                    \
      syslog(LOG_INFO, fmt, ##__VA_ARGS__);                     \
    }                                                           \
  } while (0)

#endif  // !DEBUG_H
