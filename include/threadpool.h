#ifndef __LAJI_THREAD_POOL
#define __LAJI_THREAD_POOL

#include <pthread.h>

typedef struct laji_pool_task {
    void* (*callback_func)(void*); // func ptr
    void* arg;
    struct laji_pool_task *next;
} pool_task_t;

typedef struct laji_threadpool {
    int shutdown;
    int max_thread_count;
    pthread_t *thread_id_arr;
    pool_task_t *task_chain; // chain/queue head
    pthread_mutex_t chain_mutex;
    pthread_cond_t chain_cond;
} taskpool_t;

int laji_pool_init(int max_thread_cnt);
int laji_pool_close();
int laji_task_create(void* (*callback_func)(void*), void* arg);

#endif /* __LAJI_THREAD_POOL */