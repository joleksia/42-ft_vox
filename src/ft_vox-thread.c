#include "ft_vox.h"

/* static API */

static int __ftThreadPoolPop(struct ThreadPool *self, struct job_node_s **pNode) {
    /* null-check */
    if (!self) { return (0); }

    /* lock 'mutex' */
    SDL_LockMutex(self->mutex);

    /* pop 'head' */
    struct job_node_s *head = self->jobs.head;
    switch (self->jobs.size) {
        case (0): break; 
        case (1): {
            self->jobs.head = self->jobs.tail = 0;
            self->jobs.size = 0;
        } break;

        default: {
            self->jobs.head = head->next;
            self->jobs.size--;

        } break;
    }

    /* unlock 'mutex' */
    SDL_UnlockMutex(self->mutex);

    /* return 'head' */
    if (pNode) { *pNode = head;}

    /* success */
    return (1);
}


static int __ftThreadPoolUpdate(void *data) {
    /* null-check */
    struct ThreadPool *self = (struct ThreadPool *) data;
    if (!self) { return (0); }

    SDL_LockMutex(self->mutex);
    while (1) {
        /* until 'running' */
        if (self->running) {
            /* wait for any job */
            if (!self->jobs.head) {
                SDL_CondWait(self->cond, self->mutex);
            }
        }
        /* otherwise 'break' */
        else {
            break;
        }
        
        /* pop the 'node' */
        struct
        job_node_s *node = 0;
        __ftThreadPoolPop(self, &node);
        
        self->working++;
        SDL_UnlockMutex(self->mutex);
    
        /* exec. 'node' */
        if (node) {
            node->func(node->data);
            free(node);
        }

        SDL_LockMutex(self->mutex);
        self->working--;
        SDL_UnlockMutex(self->mutex);
    }
    SDL_UnlockMutex(self->mutex);

    /* success */
    return (1);
}

/* public API */

extern int ftThreadPoolCreate(struct ThreadPool *self) {
    /* null-check */
    if (!self) { return (0); }
    memset(self, 0, sizeof(*self));

    /* create 'mutex' */
    SDL_mutex *mutex = SDL_CreateMutex();
    if (!mutex) { return (0); }

    self->mutex = mutex;

    /* create 'cond' */
    SDL_cond *cond = SDL_CreateCond();
    if (!cond) {
        ftThreadPoolDelete(self);
        return (0);
    }

    self->cond = cond;

    /* create 'threads' */
    int size = SDL_GetCPUCount();
    if (size > 1) {
        /* leave single core ('main' thread) */
        size--;
    } else {
        /* use at least one core ('main' thread) */
        size = 1;
    }

    SDL_Thread **threads = (SDL_Thread **) calloc(size, sizeof(SDL_Thread *));
    if (!threads) {
        ftThreadPoolDelete(self);
        return (0);
    }
    
    self->threads.data = threads;
    self->threads.size = size;

    /* initialize each 'thread' */
    self->running = 1;
    for (int i = 0; i < size; i++) {
        SDL_Thread *thread = SDL_CreateThread((SDL_ThreadFunction) __ftThreadPoolUpdate, "ft_vox-worker", self);
        if (!thread) {
            ftThreadPoolDelete(self);
            return (0);
        }

        threads[i] = thread;
    }

    /* success */
    return (1);
}


extern int ftThreadPoolDelete(struct ThreadPool *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* refereneces */
    SDL_mutex *mutex = self->mutex;
    SDL_cond  *cond  = self->cond;

    /* announce 'shutdown' */
    if (mutex) {
        SDL_LockMutex(self->mutex);
        
        self->running = 0;
        if (cond) {
            SDL_CondBroadcast(cond);
        }
        
        SDL_UnlockMutex(self->mutex);
    }

    /* release 'threads' */
    SDL_Thread **threads = self->threads.data;
    if (threads) {
        for (size_t i = 0; i < self->threads.size; i++) {
            SDL_Thread *thread = threads[i];
            if (thread) {
                SDL_WaitThread(thread, 0);
            }
        }
        free(threads);
    }

    /* release 'cond' */
    if (cond) { SDL_DestroyCond(cond); }

    /* release 'mutex' */
    if (mutex) { SDL_DestroyMutex(mutex); }

    /* success */
    return (1);
}


extern int ftThreadPoolQueue(struct ThreadPool *self, void *data, int (*func) (void *)) {
    /* null-check */
    if (!self) { return (0); }

    /* lock 'mutex' */
    SDL_LockMutex(self->mutex);

    /* create 'node' */
    struct
    job_node_s *node = (struct job_node_s *) malloc(sizeof(struct job_node_s));
    if (!node) {
        SDL_UnlockMutex(self->mutex);
        return (0);
    }

    /* populate 'node' */
    node->next = 0;
    node->data = data;
    node->func = func;

    /* add 'node' to 'jobs' */
    switch (self->jobs.size) {
        case (0): {
            self->jobs.size = 1;
            self->jobs.head = self->jobs.tail = node;
        } break;

        default: {
            self->jobs.size++;
            self->jobs.tail->next = node;
            self->jobs.tail = node;
        } break;
    }

    /* unlock 'mutex' */
    SDL_CondSignal(self->cond);
    SDL_UnlockMutex(self->mutex);

    /* success */
    return (1);
}
