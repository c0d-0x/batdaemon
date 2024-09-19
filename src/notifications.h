#ifndef NOTIFY_H
#define NOTIFY_H
#include <libnotify/notification.h>
#include <libnotify/notify.h>

#include "logger.h"

extern NotifyNotification *notify_instance;

void initialize_notify(void);
void cleanup_notify();
void notify_send_msg(proc_info_t *, NotifyNotification *);
#endif  // !NOTIFY_H
