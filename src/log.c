#include "log.h"

static FILE *log_handle = NULL;
static char log_path_storage[PATH_MAX];

static void log_vwrite(const char *level, const char *fmt, va_list args)
{
    if (!log_handle) {
        return;
    }

    time_t raw = time(NULL);
    struct tm tm_info;
    localtime_r(&raw, &tm_info);

    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_info);

    fprintf(log_handle, "[%s] %-5s ", timestamp, level);
    vfprintf(log_handle, fmt, args);
    fputc('\n', log_handle);
    fflush(log_handle);
}

int log_init(const char *path)
{
    const char *target = path ? path : INSTALL_LOG_PATH;
    snprintf(log_path_storage, sizeof(log_path_storage), "%s", target);

    log_handle = fopen(log_path_storage, "a");
    if (!log_handle) {
        fprintf(stderr, "Failed to open log file %s: %s\n", log_path_storage, strerror(errno));
        return -1;
    }

    setvbuf(log_handle, NULL, _IOLBF, 0);
    log_info("%s %s starting", INSTALLER_NAME, INSTALLER_VERSION);
    return 0;
}

void log_close(void)
{
    if (log_handle) {
        log_info("%s shutting down", INSTALLER_NAME);
        fclose(log_handle);
    }
    log_handle = NULL;
}

void log_info(const char *fmt, ...)
{
    if (!log_handle) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    log_vwrite("INFO", fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...)
{
    if (!log_handle) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    log_vwrite("ERROR", fmt, args);
    va_end(args);
}

const char *log_get_path(void)
{
    return log_path_storage;
}
