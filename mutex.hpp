#ifndef OSL_MUTEX_HPP
#define OSL_MUTEX_HPP

#include "config.hpp"

#if defined(OSL_IMPLEMENTATION_CXX11)

#include <mutex>
namespace osl
{
using std::mutex;
} // namespace osl

#elif defined(OSL_IMPLEMENTATION_CMSIS)

#include "cmsis/mutex.hpp"

#else

#error "No known implementation for the OS layer."

#endif

#endif // OSL_MUTEX_HPP
