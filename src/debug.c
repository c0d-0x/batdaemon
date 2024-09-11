#include "debug.h"
#include "logger.h"
#include <string.h>
#include <time.h>
void DEBUG(char *debug_msg, char *debug_msg2) {
  if (debug) {
    char time_str[26];
    get_locale_time(time_str);
    time_str[19] = '\0';
    fprintf(stdout, "%s %s", &time_str[11], debug_msg);
    if (debug_msg2 != NULL) {
      fprintf(stdout, "%s\n", debug_msg2);
    }
  }
}
