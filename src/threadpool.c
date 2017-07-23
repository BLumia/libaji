#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"

taskpool_t *laji_taskpool = NULL;

void* laji_thread_idlework(void *arg);

int laji_pool_init(int max_thread_cnt) {

    laji_taskpool = calloc(1, sizeof(taskpool_t));
    laji_taskpool->max_thread_count = max_thread_cnt;
    laji_taskpool->shutdown = 0;
    laji_taskpool->task_chain = NULL;
    pthread_mutex_init(&laji_taskpool->chain_mutex, NULL);
    pthread_cond_init(&laji_taskpool->chain_cond, NULL);
    laji_taskpool->thread_id_arr = calloc(max_thread_cnt, sizeof(pthread_t));

    // prepare threads
    for(int i = 0; i < max_thread_cnt; i++) {
        if (pthread_create(&laji_taskpool->thread_id_arr[i], NULL, laji_thread_idlework, NULL) != 0) {
            // laji_log("Cannot create more threads.");
            laji_taskpool->max_thread_count = i;
            break;
        }
    }

    return 0;
}

int laji_pool_close() {

    if (laji_taskpool->shutdown) return 0;
    laji_taskpool->shutdown = 1;

    pthread_mutex_lock(&laji_taskpool->chain_mutex);
    pthread_cond_broadcast(&laji_taskpool->chain_cond);
    pthread_mutex_unlock(&laji_taskpool->chain_mutex);

    for (int i = 0; i < laji_taskpool->max_thread_count; ++i) {
        pthread_join(laji_taskpool->thread_id_arr[i], NULL);
    }

    pool_task_t *tmp_task_ptr;
    free(laji_taskpool->thread_id_arr);
    while(laji_taskpool->task_chain != NULL) {
        tmp_task_ptr = laji_taskpool->task_chain;
        laji_taskpool->task_chain = laji_taskpool->task_chain->next;
        free(tmp_task_ptr);
    }

    pthread_mutex_destroy(&laji_taskpool->chain_mutex);
    pthread_cond_destroy(&laji_taskpool->chain_cond);

    free(laji_taskpool);
}

int laji_task_create(void* (*callback_func)(void*), void* arg) {
    
    pool_task_t *task, *here; // ??

    if (!callback_func) {
        // laji_log("create thread via laji_task_create() with invalid args.");
        return -1;
    }

    task = malloc(sizeof(pool_task_t));
    task->callback_func = callback_func;
    task->arg = arg;
    task->next = NULL;

    // put thread in thread/task chain/pool
    pthread_mutex_lock(&laji_taskpool->chain_mutex);
    if (laji_taskpool->task_chain == NULL) {
        laji_taskpool->task_chain = task;
    } else {
        here = laji_taskpool->task_chain;
        while (here->next != NULL) {
            here = here->next;
        }
        here->next = task;
    }
    
    pthread_cond_signal(&laji_taskpool->chain_cond);
    pthread_mutex_unlock(&laji_taskpool->chain_mutex);
    
    return 0;
}

void* laji_thread_idlework(void *arg) {

    for(;;) {

        pthread_mutex_lock(&laji_taskpool->chain_mutex);

        while(!laji_taskpool->task_chain && !laji_taskpool->shutdown) {
            // "There is nothing to do."
            pthread_cond_wait(&laji_taskpool->chain_cond, &laji_taskpool->chain_mutex);
        }
        if (laji_taskpool->shutdown) {
            pthread_mutex_unlock(&laji_taskpool->chain_mutex);
            pthread_exit(NULL);
        }

        pool_task_t *task;
        task = laji_taskpool->task_chain;
        laji_taskpool->task_chain = laji_taskpool->task_chain->next;

        pthread_mutex_unlock(&laji_taskpool->chain_mutex);

        task->callback_func(task->arg);

        free(task);

    }
}