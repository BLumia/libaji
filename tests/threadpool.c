#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "threadpool.h"

void* func(void* arg) {
    printf("thread %d say hi\n", *(int*)arg);
    printf("thread %d say hi\n", *(int*)arg);
    printf("thread %d say hi\n", *(int*)arg);
    printf("thread %d say hi\n", *(int*)arg);
    free(arg);
    return NULL;
}

int main() {
    laji_pool_init(61);
    for(int i = 0; i <= 20; i++) {
        int* fucker = malloc(sizeof(int));
        *fucker = i; 
        laji_task_create(func, (void*)fucker);
    }
    sleep(3);
    laji_pool_close();
    return 0;
}
