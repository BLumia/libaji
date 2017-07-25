#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "logger.h"

void *thread_function(void *arg) {
    for (int i=0; i<20; i++) {
        laji_log('v', "Thread %d says hi!", *(int*)arg);
    }
    free(arg);
    return NULL;
}

int main() {

    pthread_t mythreads[61];

    laji_log_init("./");

    for(int i = 0; i < 20; i++) {
        int* new_i = malloc(sizeof(int));
        *new_i = i;
        pthread_create(&mythreads[i], NULL, thread_function, (void*)new_i);
    }

    sleep(2);
    
    laji_log_s(LOG_VERBOSE, "done");
    laji_log_close();

}
