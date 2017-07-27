#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "logger.h"

int laji_log_findnewfile();
void * laji_log_mqhandler(void *arg);

char laji_l2c[] = {'?', 'V', 'D', 'I', 'W', 'E', 'A', 'M'};

pthread_mutex_t logger_mutex = PTHREAD_MUTEX_INITIALIZER;
char laji_log_filepath[616]; // right way to save file path?
int laji_log_inited = 0;
int laji_log_filefd = -1;
int laji_log_enabled = 1;
volatile int laji_log_mq_enabled = 0;
log_level_t laji_log_level = 0;
struct tm* laji_log_today;
pthread_t laji_log_mqhandler_p;
mqd_t laji_log_mqdes;

int laji_log_init(const char* path) {
    if (laji_log_enabled) {
        pthread_mutex_init(&logger_mutex, NULL);
        strcpy(laji_log_filepath, path);
        if (laji_log_findnewfile() != -1) {
            laji_log_inited = 1;
            return 0;
        } else {
            fputs("laji_log_init() can not create file for logger.\n", stdout);
        }
    }
    return 0;
}

int laji_log_level_set(log_level_t loglevel) {
    laji_log_level = loglevel;
    return 0;
}

int laji_log_level_set_c(char charval) {
    for (int i = 1; i <= 7; i++) {
        if (charval == laji_l2c[i]) {
            laji_log_level = i;
            return 0;
        }
    }
    return -1;
}

void * laji_log_mqhandler(void *arg) {

    struct mq_attr attr;
    ssize_t recvlen;
    laji_logmq_msg_t* msg;
    void *buf;
    int should_stop = 0;

    // setting mq
    laji_log_mqdes = mq_open("/lajihttpd_logger_mq", O_RDWR | O_CREAT | O_EXCL, 0666, NULL);
    if (laji_log_mqdes < 0) {
        if (errno == EEXIST) {
            mq_unlink("/lajihttpd_logger_mq");
            laji_log_mqdes = mq_open("/lajihttpd_logger_mq", O_RDWR | O_CREAT, 0666, NULL);
        } else {
            perror("laji_log_mqhandler()");
        }
    }

    if (mq_getattr(laji_log_mqdes, &attr) == -1) perror("mq_getattr");
    buf = malloc(attr.mq_msgsize);
    if (buf == NULL) perror("malloc");

    for(;;) {
        while ((recvlen = mq_receive(laji_log_mqdes, buf, attr.mq_msgsize, NULL)) >= 0) {
            msg = buf;
            if (msg->stop_signal == 1) {
                laji_log_s(msg->log_level, "Stopping Logger Message Queue...");
                should_stop = 1;
                break;
            }
            laji_log_s(msg->log_level, msg->buffer);
        }
        if (should_stop == 1) break;
        if (errno != EAGAIN) perror("mq_receive()");
    }

    free(buf);
    mq_close(laji_log_mqdes);
    mq_unlink("/lajihttpd_logger_mq");
    laji_log_mq_enabled = 0;

    return NULL;
}

int laji_log_mq_toggle(int enable_mq) {

    if (laji_log_enabled == 0) return 0;

    enable_mq = enable_mq == 0 ? 0 : 1;
    if (laji_log_mq_enabled == enable_mq) return 0;

    if (enable_mq) {
        pthread_create(&laji_log_mqhandler_p, NULL, laji_log_mqhandler, NULL);
        laji_log_mq_enabled = 1;
    } else {
        laji_logmq_msg_t* buffer = malloc(sizeof(laji_logmq_msg_t));
        buffer->log_level = LOG_INFO;
        buffer->stop_signal = 1;
        if (mq_send(laji_log_mqdes, (const char*)buffer, sizeof(laji_logmq_msg_t), 2) < 0) {
            perror("mq_send()");
        }
        // when laji_log_mqhandler() received a shutdown msg (see ↑ here),
        // laji_log_mq_enabled will be set to zero inside laji_log_mqhandler()
    }

    return 0;
}

int laji_log_findnewfile() {
    char buffer[616], num_buffer[6], time_buffer[61];
    time_t t;

    if (laji_log_filefd != -1) close(laji_log_filefd);
    
    time(&t);
    laji_log_today = localtime(&t);
    strftime(time_buffer, 61, "%Y-%m-%d", laji_log_today);

    for(int number = 0; number <= 616; number++) {
        sprintf(num_buffer, "_%d", number);
        sprintf(buffer, "%slog_%s%s.log",
                laji_log_filepath, time_buffer, number == 0 ? "" : num_buffer);
        laji_log_filefd = open(buffer, O_WRONLY | O_CREAT | O_APPEND, 00644);
        if (laji_log_filefd == -1) {
            perror("laji_log_findnewfile()");
            return -1;
        }
        struct stat file_stat;
        fstat(laji_log_filefd, &file_stat);
        if (file_stat.st_size < 1024 * 1024 * 50) break;
    }
    return 0;
}

int laji_log(log_level_t log_level, const char *format, ...) {

    if (laji_log_enabled == 0) return 0;
    if (log_level < laji_log_level) return 0;

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    if (laji_log_mq_enabled) {
        laji_logmq_msg_t* msg = malloc(sizeof(laji_logmq_msg_t));
        strcpy(msg->buffer, buffer);
        msg->log_level = log_level;
        if (mq_send(laji_log_mqdes, (const char*)msg, sizeof(laji_logmq_msg_t), 1) < 0) {
            perror("mq_send()");
        }
        free(msg);
    } else {
        laji_log_s(log_level, buffer);
    }
    return 0;
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

    if (laji_log_mq_enabled) {
        laji_log_mq_toggle(0);
    }

    while (laji_log_mq_enabled) {
        // do nothing, block to wait `laji_log_mq_enabled` set to zero.
        sleep(1); // neccessary?
    }
    
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
