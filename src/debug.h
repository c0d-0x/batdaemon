
#ifndef DEBUG_H
#include <stdio.h>
extern size_t debug;

void DEBUG(char *debug_msg, char *debug_msg2);
/*#define DEBUG(fmt, ...)                                        \*/
/*  do {                                                         \*/
/*    if (debug) {                                               \*/
/*      fprintf(stderr, "[filemond-debug] " fmt, ##__VA_ARGS__); \*/
/*    }                                                          \*/
/*  } while (0)*/

#endif  // !DEBUG_H
