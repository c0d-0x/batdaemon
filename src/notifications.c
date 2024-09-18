#define _DEFAULT_SOURCE
#include "notifications.h"
#include "config.h"
#include "debug.h"
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
  result[strcspn(result, "\n")] = 0; // Remove newline
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
  }
}

// from chatGPT

void notify_send_msg(proc_info_t *procinfo) {
  initialize_notify();
  char *file_name = NULL;
  char buff[32] = {0};
  file_name = strdup(strrchr(procinfo->file_path, '/'));
  printf("%s", file_name);
  if (file_name == NULL) {
    DEBUG("Failed to catch valid a file name\n", NULL);
    exit(EXIT_FAILURE);
  }

  sprintf(buff, "%s: %s", procinfo->p_event, file_name);
  NotifyNotification *notify_instance =
      notify_notification_new("Cruxfilemond Event", buff, "dialog-information");

  if (notify_instance == NULL) {
    DEBUG("Failed to create a notify instance\n", NULL);
    kill(getpid(), SIGTERM);
  } else {
    GError *error = NULL;
    // notify_notification_update(notify_instance, procinfo->p_event,
    //                            "Check cf.log for Cruxfilemond event",
    //                            "dialog-information");
    notify_notification_set_urgency(notify_instance, NOTIFY_URGENCY_NORMAL);

    if (!notify_notification_show(notify_instance, &error)) {
      if (error != NULL) {
        DEBUG("Error showing notification: %s\n", error->message);
        g_error_free(error);
      }
    }

    // Cleanup

    g_object_unref(G_OBJECT(notify_instance));
  }

  // notify_uninit();  // Uninitialize libnotify
}

void cleanup_notify() {
  if (notify_is_initted()) {
    notify_uninit();
  }
}
