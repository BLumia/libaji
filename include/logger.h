#ifndef __LAJI_LOGGER
#define __LAJI_LOGGER

#include <stdio.h>
#include <pthread.h>

typedef enum LOG_LEVEL {
    LOG_VERBOSE = 'V',
    LOG_DEBUG = 'D',
    LOG_INFO = 'I',
    LOG_WARN = 'W',
    LOG_ERROR = 'E',
    LOG_ASSERT = 'A'
} log_level_t;

int laji_log_init(const char* path);
int laji_log_s(log_level_t log_level, const char* buffer);
int laji_log(log_level_t log_level, const char *format, ...);
int laji_log_close();

#endif /* __LAJI_LOGGER */