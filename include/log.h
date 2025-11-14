#ifndef LIBERO_INSTALLER_LOG_H
#define LIBERO_INSTALLER_LOG_H

#include "common.h"

int log_init(const char *path);
void log_close(void);
void log_info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void log_error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
const char *log_get_path(void);

#endif /* LIBERO_INSTALLER_LOG_H */
