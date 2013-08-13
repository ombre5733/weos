/*****************************************************************************
**
** OS abstraction layer for ARM's CMSIS.
** Copyright (C) 2013  Manuel Freiberger
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.
**
*****************************************************************************/

#include "threads.h"

// ----=====================================================================----
//     Mutex
// ----=====================================================================----

#define CEIL_TO_MILLISEC(ts)   (   (ts)->tv_sec * 1000                         \
                                + ((ts)->tv_nsec + 999999) / 1000000)

void mtx_destroy(mtx_t* mtx)
{
    if (mtx->m_id != NULL)
        osMutexDelete(mtx->m_id);
}

int mtx_init(mtx_t* mtx, int type)
{
    mtx->m_id = NULL;

    // A mutex cannot be plain and timed at the same time.
    if ((type & mtx_plain) && (type & mtx_timed))
        return thrd_error;

    // CMSIS has only recursive timed mutexes.
    if ((type & mtx_recursive) == 0 || (type & mtx_timed) == 0)
        return thrd_error;

    mtx->m_id = osMutexCreate(osMutex(mtx->m_mutexDef));
    return (mtx->m_id != NULL) ? thrd_success : thrd_error;
}

int mtx_lock(mtx_t* mtx)
{
    osStatus status = osMutexWait(mtx->m_id, osWaitForever);
    if (status == osOK)
        return thrd_success;
    else
        return thrd_error;
}

int mtx_timedlock(mtx_t* restrict mtx, const struct timespec* restrict ts)
{
    TODO: use a uint32_t instead of the timespec struct
      OR: make the timespec somehow callable with a define e.g.
          mtx_timedlock(&myMutex, MILLISEC(100))
    osStatus status = osMutexWait(mtx->m_id, CEIL_TO_MILLISEC(ts));
    if (status == osOK)
        return thrd_success;
    else if (status == osErrorTimeoutResource)
        return thrd_timedout;
    else
        return thrd_error;
}

int mtx_trylock(mtx_t* mtx)
{
    osStatus status = osMutexWait(mtx->m_id, 0);
    if (status == osOK)
        return thrd_success;
    else if (status == osErrorResource)
        return thrd_busy;
    else
        return thrd_error;
}

int mtx_unlock(mtx_t* mtx)
{
    osStatus status = osMutexRelease(mtx->m_id);
    if (status == osOK)
        return thrd_success;
    else
        return thrd_error;
}

// ----=====================================================================----
//     Condition variable
// ----=====================================================================----

typedef struct
{
    cnd_node_t* next;
    sem_t signal;
} cnd_node_t;

int cnd_init(cnd_t* cond)
{
    cnd->queue = NULL;
    mtx_init(cond->queueMutex, mtx_plain);
}

int cnd_signal(cnd_t* cond)
{
    cnd_node_t* queue_head;
    int rc;

    rc = mtx_lock(&cond->queueMutex);
    if (rc != thrd_success)
        return thrd_error;

    queue_head = cond->queue;
    if (queue_head != NULL)
    {
        cond->queue = queue_head->next;
        osSemaphoreRelease(&queue_head->signal); // TODO: check return
    }
    rc = mtx_unlock(&cond->queueMutex);
    if (rc != thrd_success)
        return thrd_error;

    return ??;
}

int cnd_broadcast(cnd_t* cond)
{
    cnd_node_t* queue_head;

    mtx_lock(&cond->queueMutex);
    for (queue_head = cond->queue; queue_head != NULL; queue_head = queue_head->next)
    {
        osSemaphoreRelease(&queue_head->signal); // TODO: check return
    }
    cond->queue = NULL;
    mtx_unlock(&cond->queueMutex);

    return ??;
}

int cnd_wait(cnd_t* cond, mtx_t* mtx)
{
    cnd_node_t node;

    mtx_lock(&cond->queueMutex);
    // TODO: enqueue using priorities and change to a FIFO
    node->next = cond->queue;
    cond->queue = node;
    sem_init(&node->signal, 0); // TODO: check return
    mtx_unlock(&cond->queueMutex);

    // We can only unlock the mutex when we are sure that a signal will
    // reach our thread.
    mtx_unlock(mtx);

    // Wait until our semaphore receives a signal.
    int32_t tokenCount = osSemaphoreWait(&node->signal, osWaitForever); // TODO: check return
    if (tokenCount <= 0) error();

    // Re-lock the mutex.
    mtx_lock(mtx);

    return ??
}

// ----=====================================================================----
//     Thread
// ----=====================================================================----

int thrd_create(thrd_t* thr, thrd_start_t func, void* arg)
{
    // Note: CMSIS requires a thread definition object in order to create a thread.
    // This definition object also includes the maximum number of thread instances.
    // Fortunately, at least the CMSIS-RTOS implementation by Keil does not make use
    // of this parameter which is why it is set to zero here. By setting the
    // user stack size to zero, we create a thread with the default stack size.
    osThreadDef_t thread_def = { thrd_start_t, DEFAULT_PRIORITY, 0, 0 };
    *thr = osThreadCreate(&thread_def, arg);
    if (*thr != NULL)
        return thrd_success;
    else
        return thrd_error;
}

thrd_t thrd_current(void)
{
    return osThreadGetId();
}

int thrd_detach(thrd_t thr)
{
                TODO();
}

int thrd_equal(thrd_t thr0, thrd_t thrd1)
{
   return thr0 == thr1;
}

//_Noreturn void thrd_exit(int res);
#define thrd_exit(res)   return (res);

int thrd_join(thrd_t thr, int *res)
{
                TODO();
}

int thrd_sleep(const struct timespec* duration,
               struct timespec* remaining)
{
    osStatus status = osDelay(CEIL_TO_MILLISEC(duration));
    TODO: set remaining
}

void thrd_yield(void)
{
    osThreadYield();
}

//////////////////////////
////  Thread extensions
//////////////////////////

int thrdx_create_stacksize(thrd_t* thr, thrd_start_t func, void* arg, int stacksize)
{
    osThreadDef_t thread_def = { thrd_start_t, DEFAULT_PRIORITY, 0, stacksize };
    *thr = osThreadCreate(&thread_def, arg);
    if (*thr != NULL)
        return thrd_success;
    else
        return thrd_error;
}

// int thrd_native_get_priority
int thrdx_get_priority(thrd_t thr)
{
    return osThreadGetPriority(id);
}

int thrdx_set_priority(thrd_t thr, thrdx_priority_t priority)
{
    osStatus status = osThreadSetPriority(thr, priority);
    if (status == osOK)
        return thrd_success;
    else
        return thrd_error;
}
