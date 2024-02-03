#include "daemon.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char *load_file(char *file_Path) {
  char *saveptr, *temp;
  char *file_name = malloc(sizeof(uint16_t *));
  temp = strtok_r(file_Path, "/", &saveptr);
  while (temp != NULL) {
    file_name = temp;
    temp = strtok_r(NULL, "/", &saveptr);
  }
  if (file_name != NULL && file_Path != NULL) {
    return file_name;
  }

  return NULL;
}
