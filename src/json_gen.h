#ifndef JSON_GEN_H
#define JSON_GEN_H

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*10 MB max */
#define FILE_SIZE_MAX 10485760
/*{}\r\n: json obj file or []\r\n: json array file*/
#define BEGIN_SYMBOL "[]\r\n"
enum FILE_STATE { EMPTY_FILE = 0, NOT_FOUND, VALID_JSON, INVALID_JSON };

typedef struct {
  char *date;
  char *file;
  char *e_process;
  char *e_p_event;
  char *e_username;
  char *e_p_Umask;
  char *e_p_state;
} json_obj_t;

#define OFFSET 3

#define CUSTOM_ERR (-1)
#define JSON_FILE "cf_log.json"

/*10 MB max */
#define FILE_SIZE_MAX 10485760
/*{}\r\n: json obj file or []\r\n: json array file*/
#define BEGIN_SYMBOL "[]\r\n"

int rotate_json_f(FILE *json_fp, char *file_name);
void write_json_fmt(FILE *json_fp, char *fmt, ...);
void backup_json_f(char *file_path);
size_t get_json_f_size(char *file_path);
FILE *create_new_json_f(const char *file_path, const char *begin_symbol);
FILE *init_json_gen(void);
void close_json_f(FILE *json_fp);

#endif  // !JSON_GEN_H
