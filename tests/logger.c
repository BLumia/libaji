#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "logger.h"

void *thread_function(void *arg) {
    char buffer[61];
    sprintf(buffer, "Thread %d says hi!", *(int*)arg);
    for (int i=0; i<20; i++) {
        laji_log(buffer);
    }
    return NULL;
}

int main() {

    pthread_t mythreads[61];

    laji_log_init("./");

    for(int i = 0; i < 20; i++) {
        pthread_create(&mythreads[i], NULL, thread_function, (void*)&i);
    }

    sleep(2);
    
    laji_log("done");
    laji_log_close();

}
