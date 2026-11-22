#include "ft_vox.h"

/* static API */

static int __ft_thpool_pop(struct thpool_s *self, struct job_node_s **pNode) {
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


static int __ft_thpool_update(void *data) {
    /* null-check */
    struct thpool_s *self = (struct thpool_s *) data;
    if (!self) { return (0); }

    while (1) {
        SDL_LockMutex(self->mutex);
        
        /* wait for upcoming jobs... */
        while (!self->jobs.head) {
            /* break if no longer 'alive' */
            if (!SDL_AtomicGet(&self->alive)) {
                break;
            }

            SDL_CondWait(self->cond, self->mutex);
        }

        /* break if no longer 'alive' */
        if (!SDL_AtomicGet(&self->alive)) {
            SDL_UnlockMutex(self->mutex);
            break;
        }
        
        /* pop the 'node' */
        struct
        job_node_s *node = 0;
        __ft_thpool_pop(self, &node);

        SDL_UnlockMutex(self->mutex);
    
        /* exec. 'node' */
        if (node) {
            void *func = node->func;
            void *data = node->data;
            if (func) {
                ((thread_pool_func_t) func) (data);
            }
            
            /* release 'node' */
            free(node);
        }
    }

    /* success */
    return (1);
}

/* public API */

extern int ft_thpool_create(struct thpool_s **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'thpool' */
    struct thpool_s *thpool = (struct thpool_s *) calloc(1, sizeof(struct thpool_s));
    if (!thpool) { return (0); }

    /* create 'mutex' */
    SDL_mutex *mutex = SDL_CreateMutex();
    if (!mutex) { return (0); }

    thpool->mutex = mutex;

    /* create 'cond' */
    SDL_cond *cond = SDL_CreateCond();
    if (!cond) {
        ft_thpool_delete(thpool);
        return (0);
    }

    thpool->cond = cond;

    /* create 'threads' */
    int size = SDL_GetCPUCount();
    if (size > 0) {
        size -= 1;
    } else {
        size = 1;
    }

    SDL_Thread **threads = (SDL_Thread **) calloc(size, sizeof(SDL_Thread *));
    if (!threads) {
        ft_thpool_delete(thpool);
        return (0);
    }
    
    thpool->threads.data = threads;
    thpool->threads.size = size;

    /* initialize each 'thread' */
    SDL_AtomicSet(&thpool->alive, 1);
    for (int i = 0; i < size; i++) {
        threads[i] = SDL_CreateThread((SDL_ThreadFunction) __ft_thpool_update, "ft_vox-worker", thpool);
    }

    /* return 'self' */
    *self = thpool;

    /* success */
    return (1);
}


extern int ft_thpool_delete(struct thpool_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* refereneces */
    SDL_mutex *mutex = self->mutex;
    SDL_cond  *cond  = self->cond;

    /* announce 'shutdown' */
    if (mutex) {
        SDL_LockMutex(mutex);
        
        SDL_AtomicSet(&self->alive, 0);
        if (cond) {
            SDL_CondBroadcast(cond);
        }
        
        SDL_UnlockMutex(mutex);
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

    /* release leftover 'jobs' */
    SDL_LockMutex(mutex);
    while (self->jobs.size > 0) {
        /* get job 'node' */
        struct
        job_node_s *node = 0;
        __ft_thpool_pop(self, &node);

        /* release 'node' */
        free(node);
    }
    SDL_UnlockMutex(mutex);

    /* release 'cond' */
    if (cond) { SDL_DestroyCond(cond); }

    /* release 'mutex' */
    if (mutex) { SDL_DestroyMutex(mutex); }

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_thpool_queue(struct thpool_s *self, void *data, int (*func) (void *)) {
    /* null-check */
    if (!self) { return (0); }

    /* lock 'mutex' */
    SDL_LockMutex(self->mutex);

    /* create 'node' */
    struct
    job_node_s *node = (struct job_node_s *) calloc(1, sizeof(struct job_node_s));
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


extern int ft_thpool_dequeue(struct thpool_s *self, void *data) {
    /* null-check */
    if (!self) { return (0); }
    if (!data) { return (0); }

    int drop = 0;
    
    /* lock 'mutex' */
    SDL_LockMutex(self->mutex);

    /* iterate over each node in 'pool' */
    struct
    job_node_s *curr = self->jobs.head,
               *next = 0,
               *prev = 0;
    while (curr) {
        next = curr->next;

        /* case : 'data' found */
        if (curr->data == data) {
            /* re-position 'head' */
            if (prev) {
                prev->next = next;
            } else {
                self->jobs.head = next;
            }

            /* re-position 'tail' */
            if (self->jobs.tail == curr) {
                self->jobs.tail = prev;
            }

            /* release 'curr' job */
            self->jobs.size--; drop++;
            free(curr);
        }

        /* case : 'data' not found */
        else {
            prev = curr;
        }

        curr = next;
    }

    /* unlock 'mutex' */
    SDL_UnlockMutex(self->mutex);

    /* success */
    return (drop);
}
