#ifndef NOTIFY_H
#define NOTIFY_H

#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <sys/types.h>
void initialize_notify(char* appname, char* icon, size_t expires);
void notify_send_msg(const char*, const char*, ssize_t ugency);
void cleanup_notify(void);
void close_notification(void);
#endif  // !NOTIFY_H
