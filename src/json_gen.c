#include "json_gen.h"

#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core.h"
#include "debug.h"
#include "logger.h"
#include "main.h"

size_t validate_json(char *json_file) {
  char CC;
  int len, json_fd = -1;
  if ((json_fd = open(json_file, O_RDONLY)) == -1) {
    DEBUG("Failed to open json file");
    kill(getpid(), SIGTERM);
  }

  len = read(json_fd, &CC, sizeof(char));
  if (len == -1) {
    close(json_fd);
    DEBUG("Error: read failed: %s", strerror(errno));
    kill(getpid(), SIGTERM);
  }

  if (CC != '[') {
    close(json_fd);
    if (len == 0)
      return EMPTY_FILE;
    else
      return INVALID_JSON;
  }

  /**
   * seeking to the last three bytes of the file, which normally should be
   * ']\r\n' -> This is necessary for json decoders
   */
  lseek(json_fd, -(long)sizeof(char) * 3, SEEK_END);
  len = read(json_fd, &CC, sizeof(char));
  if (len < 1) {
    close(json_fd);
    DEBUG("Error: read failed: %s", strerror(errno));
    kill(getpid(), SIGTERM);
  }

  close(json_fd);
  if (CC != ']') return INVALID_JSON;
  return VALID_JSON;
}

/*functions constructs the json object.*/
void json_constructor(FILE *json_fp, json_obj_t *json_obj) {
  if (json_fp != NULL) {
    fprintf(json_fp, "{\"date\":\"%s\",", json_obj->date);
    fprintf(json_fp, "\"file\":\"%s\",", json_obj->file);
    fprintf(json_fp, "\"e_process\":\"%s\" ,", json_obj->e_process);
    fprintf(json_fp, "\"e_p_event\":\"%s\",", json_obj->e_p_event);
    fprintf(json_fp, "\"e_p_state\":\"%s\",", json_obj->e_p_state);
    fprintf(json_fp, "\"e_p_Umask\":\"%s\",", json_obj->e_p_Umask);
    fprintf(json_fp, "\"e_username\":\"%s\"},", json_obj->e_username);
  }
}

void write_json_obj(FILE *json_fp, json_obj_t *json_obj,
                    void (*json_constructor)(FILE *, json_obj_t *)) {
  json_constructor(json_fp, json_obj);
}

void close_json_file(FILE *json_fp) {
  fseek(json_fp, -(long)sizeof(char), SEEK_END);
  fwrite("]\r\n", sizeof(char) * 3, 1, json_fp);
  if (json_fp) fclose(json_fp);
}

size_t get_file_size(char *file_path) {
  struct stat buf;
  if (stat(file_path, &buf) != 0) {
    DEBUG("Error: File  stats failed: %s", strerror(errno));
    return CUSTOM_ERR;
  }
  return buf.st_size;
}

void backup_file(char *file_path) {
  char buffer[64];
  if (access(file_path, F_OK) != 0) return;

  char *time_date = get_locale_time();
  snprintf(buffer, 63, "%s.json", time_date);
  rename(file_path, buffer);
  if (!time_date) free(time_date);
}

FILE *create_new_log(char *file_path) {
  FILE *fp = NULL;
  if ((fp = fopen(file_path, "w")) == NULL) {
    DEBUG("Error : Failed to create_new_log: %s", strerror(errno));
    kill(getpid(), SIGTERM);
  }
  fputc('[', fp);
  return fp;
}

void init_json_gen(FILE *json_fp) {
  size_t flag = validate_json(LOG_FILE);
  ssize_t file_size;
  switch (flag) {
    case VALID_JSON:
      file_size = get_file_size(LOG_FILE);
      if (file_size >= FILE_SIZE_MAX) {
        close_json_file(json_fp);
        backup_file(LOG_FILE);
        json_fp = create_new_log(LOG_FILE);
        return;
      }
      fseek(json_fp, -(long)sizeof(char) * 3, SEEK_END);
      fwrite(",", sizeof(char), 1, json_fp);
      break;
    case EMPTY_FILE:
      fputc('[', json_fp);
      break;
    default:
      if (flag == NOT_FOUND) {
        DEBUG("%s: Not found", LOG_FILE);
      } else {
        DEBUG("Invalid json format: %s", LOG_FILE);
        backup_file(LOG_FILE);
      }
      json_fp = create_new_log(LOG_FILE);
      break;
  }
}
