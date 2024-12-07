#include "json_gen.h"

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "debug.h"
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
    fprintf(json_fp, "\"e_username\":\"%s\"}]\r\n", json_obj->e_username);
  }
}

static void writer_json_obj(FILE *json_fp, json_obj_t *json_obj,
                            void (*json_constructor)(FILE *, json_obj_t *)) {
  json_constructor(json_fp, json_obj);
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
      DEBUG("%s: Not found", LOG_FILE);
      kill(getpid(), SIGTERM);
      break;
    case INVALID_JSON:
      DEBUG("Invalid json format: %s", LOG_FILE);
      kill(getpid(), SIGTERM);
  }
}
