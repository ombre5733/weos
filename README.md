# weos

A modern C++ threading library for embedded development.

## Overview

WEOS is a C++ interface for ARM's CMSIS RTOS guided by the ISO-standardized
C++-STL interface. RTOS features which are covered by the standard use the
standard API (e.g. `recursive_timed_mutex`, `thread`).
Features which do not have a standard counterpart (semaphores, thread signals,
memory pools) are provided by extending the STL API.
As usual for C++, their interfaces are strictly type safe, have a low RAM
overhead and are safe to use.
Finally, WEOS provides some STL constructs if they increase the application's
performance (e.g. `atomic<>`), make the application code safer (e.g. `mutex`,
`unique_lock<>`, `lock()`) or speed up the development (e.g.
`condition_variable`, `future<>`).
However, porting the full C++STL is out of scope.

When necessary, WEOS implements part of the C++ STL (`function<>`, `tuple<>`...)
in order to be usable with both GCC and ARMCC. These parts should be considered
as transitional and will be removed when ARMCC ships with an up-to-date STL.

## Advantages

Users of WEOS will get the following benefits:

* Type safety: Call threads with zero to 100 (or even more) arguments. Pass the
  arguments by value, by reference or by pointer on your choice. Incompatible
  arguments trigger a compiler error which safes hours of debugging. No need to
  cast to or from `void*` any longer.
* Expressive interfaces: All the modern C++ functionality is available for
  embedded programming: Class member-functions or lambda-functions can be
  threads. The result of an asynchronous operation (possibly an exception)
  can be forwarded with futures. All timing functionality is guaranteed to
  be free from overflows.
* Stable interfaces: Nearly all of the functionality is covered by the ISO
  standard and is highly unlikely to change within the next 10 years.
* Seamless upgrading to new compiler features. In fact, WEOS supports much
  of C++14 right now. Support for C++17 (e.g. resumable functions) is work
  in progress and should be transparent to the user code.

## Contents

### Threading support:
* `std::thread`   [including thread attributes and RTOS signal support]
* `std::async()`
* `std::future<>`
* `std::promise<>`

* `std::sleep_for()`
* `std::sleep_until()`

* `std::mutex`
* `std::timed_mutex`
* `std::recursive_mutex`
* `std::recursive_timed_mutex`
* `std::lock_guard<>`
* `std::unique_lock<>`
* `std::lock()`       [missing overloads for more than two lockables]
* `std::try_lock()`   [missing overloads for more than two lockables]

* `std::condition_variable`

### Chrono support:
* `std::duration<>`
* `std::duration_cast<>()`
* `std::time_point<>`
* `std::high_resolution_clock`
* `std::steady_clock`
* `std::system_clock`

### Atomic operations support:
* `std::atomic_thread_fence()`
* `std::atomic_flag`
* `std::atomic<T>`
* `std::atomic<T*>`
* `std::atomic<bool>`

### Extensions

The following features are not covered by the ISO standard but provided
nonetheless as they are important for embedded development:
* thread signals
* thread attributes for priorities and stack sizes
* semaphore
* message queues for inter-thread communication
* object pools for type-safe pool allocation
* shared object pools for thread-safe pool allocation

Additionally these low-level constructs without type safety are provided but
should be avoided in favor of their type-safe counterparts:
* memory pools (similar to object pools but with an unsafe `void*` interface)
* shared memory pool (similar to shared object pools but with an unsafe `void*`
  interface)
