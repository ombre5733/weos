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

#ifndef OSL_CMSIS_TIMER_HPP
#define OSL_CMSIS_TIMER_HPP

#include "cmsis_os.h"

#include <boost/utility.hpp>

namespace osl
{

//! A timer.
//! A timer allows the execution of a callback after a defined timeout.
class timer : boost::noncopyable
{
public:
    enum Type
    {
        OneShot = osTimerOnce,
        Periodic = osTimerPeriodic
    };

    timer(Type type, void (*callback)(void*), void* argument)
        : m_timerId(0)
    {
        osTimerDef_t timerDef = { callback, m_cmsisTimerControlBlock };
        m_timerId = osTimerCreate(&timerDef, type, argument);
    }

    ~timer()
    {
        if (m_timerId)
            osTimerDelete(m_timerId);
    }

    void timer::start()
    {
        osStatus status = osTimerStart(m_timerId, 100/*! \todo: Time */);
        assert(status == osOK);
    }

    void stop()
    {
        osStatus status = osTimerStop(m_timerId);
        //! osErrorResource indicates that the timer has not been started, yet.
        //! Should we allow this?
        assert(status == osOK || osErrorResource);
    }

private:
    uint32_t m_cmsisTimerControlBlock[5];
    osTimerId m_timerId;
};

} // namespace osl

#endif // OSL_CMSIS_TIMER_HPP
