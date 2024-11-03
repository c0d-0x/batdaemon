#include "json_gen.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "filemond.h"
#include "main.h"

size_t validate_json(char *json_file) {
  char CC;
  FILE *json_fp = NULL;
  if ((json_fp = fopen(json_file, "r")) == NULL) {
    fprintf(stderr, "Failed to open json file: %s", strerror(errno));
    return NOT_FOUND;
  }

  CC = fgetc(json_fp);
  if (CC != '[') {
    fclose(json_fp);
    if (CC == EOF)
      return EMPTY_FILE;
    else
      return INVALID_JSON;
  }

  /**
   * seeking to the last three bytes of the file, which normally should be
   * ']\r\n' -> This is necessary for json decoders
   */
  fseek(json_fp, -(long)sizeof(char) * 3, SEEK_END);
  CC = fgetc(json_fp);
  fclose(json_fp);

  if (CC != ']') return INVALID_JSON;

  return VALID_JSON;
}
/*functions from my json_generator lib.*/
void json_constructor(FILE *json_fp, json_obj_t *json_obj) {
  if (json_fp != NULL) {
    fputs("{", json_fp);
    fprintf(json_fp, "\"date\":\"%s\",", json_obj->date);
    fprintf(json_fp, "\"file\":\"%s\",", json_obj->file);
    fprintf(json_fp, "\"e_process\":\"%s\" ,", json_obj->e_process);
    fprintf(json_fp, "\"e_p_event\":\"%s\",", json_obj->e_p_event);
    fprintf(json_fp, "\"e_p_state\":\"%s\",", json_obj->e_p_state);
    fprintf(json_fp, "\"e_username\":\"%s\"", json_obj->e_username);
    fputs("}", json_fp);
  }
}

static void writer_json_obj(FILE *json_fp, json_obj_t *json_obj,
                            void (*json_constructor)(FILE *, json_obj_t *)) {
  json_constructor(json_fp, json_obj);
  fputs("]\r\n", json_fp);
}

void append_to_file(FILE *json_fp, json_obj_t *json_obj,
                    void (*json_constructor)(FILE *, json_obj_t *)) {
  size_t flag = validate_json(LOG_FILE);
  switch (flag) {
    case VALID_JSON:
      fseek(json_fp, -(long)sizeof(char) * 3, SEEK_END);
      fwrite(",", sizeof(char), 1, json_fp);
      writer_json_obj(json_fp, json_obj, json_constructor);
      break;
    case EMPTY_FILE:
      fputc('[', json_fp);
      writer_json_obj(json_fp, json_obj, json_constructor);
      break;
    case NOT_FOUND:
      exit(CUSTOM_ERR);
    case INVALID_JSON:
      fprintf(stderr, "Invalid json format");
      exit(CUSTOM_ERR);
  }
}
