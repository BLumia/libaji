#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "logger.h"

void *thread_function(void *arg) {
    for (int i=0; i<10; i++) {
        laji_log(LOG_VERBOSE, "Thread %d says hi!", *(int*)arg);
    }
    for (int i=0; i<10; i++) {
        laji_log(LOG_INFO, "Thread %d says hi!", *(int*)arg);
    }
    free(arg);
    return NULL;
}

int main() {

    pthread_t mythreads[61];

    laji_log_init("./");
    laji_log_level_set(LOG_INFO);

    for(int i = 0; i < 20; i++) {
        int* new_i = malloc(sizeof(int));
        *new_i = i;
        pthread_create(&mythreads[i], NULL, thread_function, (void*)new_i);
    }

    sleep(2);

    // enable POSIX MQ and standalone thread for logging
    laji_log_mq_toggle(1);

    for(int i = 21; i < 40; i++) {
        int* new_i = malloc(sizeof(int));
        *new_i = i;
        pthread_create(&mythreads[i], NULL, thread_function, (void*)new_i);
    }

    sleep(2);
    
    laji_log_s(LOG_INFO, "done");
    laji_log_close();

}
