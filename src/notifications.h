#ifndef NOTIFY_H
#define NOTIFY_H
#include "logger.h"
#include <libnotify/notify.h>

extern NotifyNotification *notify_instance;

void initialize_notify(void);
void cleanup_notify();
void notify_send_msg(proc_info_t *);
#endif // !NOTIFY_H
