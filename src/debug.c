#include "debug.h"
#include <time.h>

void DEBUG(char *debug_msg, char *debug_msg2) {
  if (debug) {
    fprintf(stdout, "DEBUG: %s", debug_msg);

    if (debug_msg2 != NULL) {
      fprintf(stdout, " %s\n", debug_msg2);
    }
  }
}
