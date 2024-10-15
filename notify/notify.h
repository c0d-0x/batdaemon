#ifndef NOTIFY_H
#define NOTIFY_H
#include <libnotify/notification.h>
#include <libnotify/notify.h>

#include "logger.h"

void initialize_notify(char* appname, char* icon, size_t expires);
void notify_send_msg(proc_info_t* procinfo, ssize_t ugency);
void cleanup_notify(void);
void close_notification(void);
#endif  // !NOTIFY_H
