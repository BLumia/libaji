#ifndef __LAJI_LOGGER
#define __LAJI_LOGGER

#include <stdio.h>
#include <pthread.h>

typedef enum LOG_LEVEL {
    LOG_VERBOSE = 1,
    LOG_DEBUG = 2,
    LOG_INFO = 3,
    LOG_WARN = 4,
    LOG_ERROR = 5,
    LOG_ASSERT = 6
} log_level_t;

int laji_log_init(const char* path);
int laji_log_level_set(log_level_t loglevel);
int laji_log_level_set_c(char charval);
int laji_log_s(log_level_t log_level, const char* buffer);
int laji_log(log_level_t log_level, const char *format, ...);
int laji_log_close();

#endif /* __LAJI_LOGGER */