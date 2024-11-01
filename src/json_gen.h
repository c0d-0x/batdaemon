#ifndef JSON_GEN_H
#define JSON_GEN_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define JSON_FILE "sample_gen.json"

enum FILE_STATE { EMPTY_FILE = 0, NOT_FOUND, VALID_JSON, INVALID_JSON };

/**
 * This supposed to be custom generated, to suite your json object.
 * Use this structure to construct your json_constructor function.
 * */
typedef struct {
  char *date;
  char *file;
  char *e_process;
  char *e_p_event;
  char *e_username;
  char *e_p_Umask;
  char *e_p_state;
} json_obj_t;

void append_to_file(FILE *json_fp, json_obj_t json_obj,
                    void (*json_constructor)(FILE *, json_obj_t));
size_t validate_json(char *json_file);

#endif  // !JSON_GEN_H
