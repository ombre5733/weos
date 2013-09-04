#ifndef WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP
#define WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP

#include "../mutex.hpp"

struct SparringData
{
    enum Action
    {
        None,
        MutexLock,
        MutexTryLock,
        MutexUnlock,
        Terminate
    };

    SparringData()
        : action(None),
          busy(false),
          mutexLocked(false),
          sparringStarted(false)
    {
    }

    weos::mutex mutex;
    volatile Action action;
    volatile bool busy;
    volatile bool mutexLocked;
    volatile bool sparringStarted;
};

extern "C" void sparring(const void* arg)
{
    SparringData* data = static_cast<SparringData*>(const_cast<void*>(arg));
    data->sparringStarted = true;

    while (1)
    {
        if (data->action == SparringData::None)
        {
            osDelay(1);
            continue;
        }
        else if (data->action == SparringData::Terminate)
            break;

        data->busy = true;
        switch (data->action)
        {
            case SparringData::MutexLock:
                data->mutex.lock();
                data->mutexLocked = true;
                break;
            case SparringData::MutexTryLock:
                data->mutexLocked = data->mutex.try_lock();
                break;
            case SparringData::MutexUnlock:
                data->mutex.unlock();
                data->mutexLocked = false;
                break;
            default:
                break;
        }
        data->busy = false;
        data->action = SparringData::None;
    }

    if (data->action == SparringData::MutexLock)
    {
        data->mutex.lock();
        data->mutexLocked = true;
    }
    else if (data->action == SparringData::MutexTryLock)
    {
        while (!data->mutexLocked)
            data->mutexLocked = data->mutex.try_lock();
    }
    while (1);
}
osThreadDef_t sparringThread = {sparring, osPriorityHigh, 1, 0};

#endif // WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP
