
#define _DEFAULT_SOURCE
#include "notifications.h"

#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "filemond.h"

// void notify_send_msg(proc_info_t *procinfo) {
//   NotifyNotification *notify_instance =
//       notify_notification_new(NULL, NULL, "dialog-information");
//
//   if (notify_instance == NULL) {
//     DEBUG("Fail to create a notify instance\n", NULL);
//     kill(getpid(), SIGTERM);
//   }
//   notify_notification_update(notify_instance, procinfo->p_event,
//                              "Check cf.log for Cruxfilemond event",
//                              "dialog-information");
//   notify_notification_set_urgency(notify_instance, NOTIFY_URGENCY_NORMAL);
//   notify_notification_show(notify_instance, NULL);
//
//   // Cleaning up
//   g_object_unref(G_OBJECT(notify_instance));
// }
//

static char *get_user_env_var(const char *username, const char *var_name) {
  char command[256] = {0x0};
  FILE *fp;
  static char result[1024] = {0x0};

  snprintf(command, sizeof(command),
           "ps -u %s -o pid= | while read pid; do tr '\\0' '\\n' < "
           "/proc/$pid/environ 2>/dev/null | grep '^%s='; done | head -n 1 | "
           "cut -d '=' -f2-",
           username, var_name);
  // bash command injection to retrieve env_var
  if ((fp = popen(command, "r")) == NULL) {
    perror("Failed to run command");
    return NULL;
  }

  if (fgets(result, sizeof(result), fp) == NULL) {
    pclose(fp);
    return NULL;
  }

  pclose(fp);
  result[strcspn(result, "\n")] = 0;  // Remove newline
  return result;
}
void initialize_notify(void) {
  if (!notify_is_initted()) {
    // Get user information
    struct passwd *pw = getpwnam(USERNAME);
    if (pw == NULL) {
      fprintf(stderr, "User %s not found.\n", USERNAME);
      exit(CUSTOM_ERR);
    }

    // Get the UID and GID of the active user
    uid_t user_uid = pw->pw_uid;
    gid_t user_gid = pw->pw_gid;

    // Get the DISPLAY and DBUS_SESSION_BUS_ADDRESS environment variables for
    // the user
    char *display = get_user_env_var(USERNAME, "DISPLAY");
    char *dbus_session = get_user_env_var(USERNAME, "DBUS_SESSION_BUS_ADDRESS");

    if (display == NULL || dbus_session == NULL) {
      fprintf(stderr, "Failed to get environment variables for user %s.\n",
              USERNAME);
      exit(CUSTOM_ERR);
    }

    // Print the extracted environment variables for debugging purposes
    printf("DISPLAY=%s\n", display);
    printf("DBUS_SESSION_BUS_ADDRESS=%s\n", dbus_session);

    // Set the environment variables for the root process
    setenv("DISPLAY", display, 1);
    setenv("DBUS_SESSION_BUS_ADDRESS", dbus_session, 1);

    // Switch to the non-root user's UID and GID
    if (setgid(user_gid) != 0 || setuid(user_uid) != 0) {
      perror("Failed to switch user");
      exit(CUSTOM_ERR);
    }

    if (!notify_init("Cruxfilemond")) {
      DEBUG("Failed to initialize libnotify\n", NULL);
      exit(EXIT_FAILURE);
    }
    notify_instance = notify_notification_new("Cruxfilemond Event", NULL,
                                              "dialog-information");
  }
  if (notify_instance == NULL) {
    fprintf(stderr, "Fail to create a notification instance\n");
    exit(CUSTOM_ERR);
  }
}

void notify_send_msg(proc_info_t *procinfo,
                     NotifyNotification *notify_instance) {
  if (notify_instance == NULL) {
    DEBUG("Failed to create a notify instance\n", NULL);
    kill(getpid(), SIGTERM);  // Terminates the process safely on failure
    return;  // Ensure function exits if kill doesn't stop the process
  }

  char *file_name = NULL;
  char buff[32] = {0};

  // Ensure that procinfo->file_path is not NULL before attempting to use it
  if (procinfo == NULL || procinfo->file_path == NULL) {
    DEBUG("Invalid procinfo or file path\n", NULL);
    kill(getpid(), SIGTERM);  // Terminates the process safely on failure
  }

  // Extract the filename from the path without modifying the original string
  file_name = strrchr(procinfo->file_path, '/');
  if (file_name == NULL) {
    DEBUG("Failed to extract a valid file name from the path\n", NULL);
    kill(getpid(), SIGTERM);  // Terminates the process safely on failure
  }

  // Securely construct the message, avoiding potential buffer overflows
  snprintf(buff, sizeof(buff), "%s: %s", procinfo->p_event, file_name + 1);

  GError *error = NULL;
  notify_notification_update(notify_instance, buff,
                             "Check cf.log for Cruxfilemond event",
                             "dialog-information");
  notify_notification_set_urgency(notify_instance, NOTIFY_URGENCY_NORMAL);

  if (!notify_notification_show(notify_instance, &error)) {
    if (error != NULL) {
      DEBUG("Error showing notification: %s\n", error->message);
      g_error_free(error);      // Free the error object
      error = NULL;             // Reset error to avoid double-free issues
      kill(getpid(), SIGTERM);  // Terminates the process safely on failure
    }
  }
}

void cleanup_notify(NotifyNotification *notify_instance) {
  if (notify_is_initted()) {
    notify_uninit();
    g_object_unref(G_OBJECT(notify_instance));
  }
}
