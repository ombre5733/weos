#ifndef WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP
#define WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP

#include "../mutex.hpp"

struct SparringData
{
    enum Action
    {
        MutexLock,
        MutexTryLock
    };

    explicit SparringData(Action a)
        : action(a),
          mutexLocked(false),
          sparringStarted(false)
    {
    }

    Action action;
    weos::mutex mutex;
    volatile bool mutexLocked;
    volatile bool sparringStarted;
};

extern "C" void sparring(const void* arg)
{
    SparringData* data = static_cast<SparringData*>(const_cast<void*>(arg));
    data->sparringStarted = true;
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
