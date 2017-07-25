#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "logger.h"

int laji_log_findnewfile();

char laji_l2c[] = {'?', 'V', 'D', 'I', 'W', 'E', 'A'};

pthread_mutex_t logger_mutex = PTHREAD_MUTEX_INITIALIZER;
char laji_log_filepath[616]; // right way to save file path?
int laji_log_inited = 0;
int laji_log_filefd = -1;
int laji_log_enabled = 1;
log_level_t laji_log_level = 0;
struct tm* laji_log_today;

int laji_log_init(const char* path) {
    if (laji_log_enabled) {
        pthread_mutex_init(&logger_mutex, NULL);
        strcpy(laji_log_filepath, path);
        if (laji_log_findnewfile() != -1) {
            laji_log_inited = 1;
            return 0;
        } else {
            fputs("laji_log_init() can not create file for read.\n", stdout);
        }
    }
    return 0;
}

int laji_log_level_set(log_level_t loglevel) {
    laji_log_level = loglevel;
    return 0;
}

int laji_log_level_set_c(char charval) {
    for (int i = 1; i <= 6; i++) {
        if (charval == laji_l2c[i]) {
            laji_log_level = i;
            return 0;
        }
    }
    return -1;
}

int laji_log_findnewfile() {
    char buffer[616], num_buffer[61], time_buffer[61];
    int number = 0;
    time_t t;

    if (laji_log_filefd != -1) close(laji_log_filefd);
    
    time(&t);
    laji_log_today = localtime(&t);
    strftime(time_buffer, 61, "%Y-%m-%d", laji_log_today);

    while(number < 616) {
        sprintf(num_buffer, "_%d", number);
        sprintf(buffer, "%slog_%s%s.log",
                laji_log_filepath, time_buffer, number == 0 ? "" : num_buffer);
        laji_log_filefd = open(buffer, O_WRONLY | O_CREAT | O_APPEND, 00644);
        if (laji_log_filefd == -1) {
            perror("laji_log_init()");
            return -1;
        }
        struct stat file_stat;
        fstat(laji_log_filefd, &file_stat);
        if (file_stat.st_size < 1024 * 1024 * 50) break;
    }
    return 0;
}

int laji_log(log_level_t log_level, const char *format, ...) {

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    laji_log_s(log_level, buffer);
}

int laji_log_s(log_level_t log_level, const char* buffer) {
    if (laji_log_enabled && laji_log_inited) {

        // check log level
        if (log_level < laji_log_level) return 0;

        pthread_mutex_lock(&logger_mutex);

        // check filesize
        struct stat file_stat;
        fstat(laji_log_filefd, &file_stat);
        if (file_stat.st_size > 1024 * 1024 * 50) laji_log_findnewfile();

        // check date
        time_t t;
        struct tm* cur_time;
        time(&t);
        cur_time = localtime(&t);
        if (cur_time->tm_yday != laji_log_today->tm_yday) laji_log_findnewfile();

        // write
        char output_buffer[616], time_buffer[61];
        int output_bufferlen;
        strftime(time_buffer, 61, "%Y-%m-%d %H:%M:%S", cur_time); 
        sprintf(output_buffer, "%s %c %s\n", time_buffer, laji_l2c[log_level], buffer);
        output_bufferlen = strlen(output_buffer);
        //06-29 18:38:32.891   824   960 I ThermalEngine: Monitor : quiet_therm = 44, msm_therm = 47, ufs_therm = 44, battery_therm = 376,current_now = 26000
        write(laji_log_filefd, output_buffer, output_bufferlen);

        pthread_mutex_unlock(&logger_mutex);
        return 0;
    } else {
        if (!laji_log_inited && laji_log_enabled) {
            // warn user, should init
            // hey but this line is not thread safe.
            // but who cares?
            fputs("laji_log need initialize!\n", stdout);
            return -1;
        } else {
            // normally disabled the logger
            return 0;
        }
    }
}

int laji_log_close() {
    if (laji_log_filefd != -1 && laji_log_enabled && laji_log_inited) {
		laji_log_inited = 0;
        close(laji_log_filefd);
    } else {
        // else ?
    }
    
    if (laji_log_enabled) {
        pthread_mutex_destroy(&logger_mutex);	
    }
}
