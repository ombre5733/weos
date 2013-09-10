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

#ifndef WEOS_CHRONO_HPP
#define WEOS_CHRONO_HPP

#include "config.hpp"

#if defined(WEOS_WRAP_CXX11)
#  include <chrono>
   namespace weos
   {
       using namespace std::chrono;
   }
#elif defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
#  include "keil_cmsis_rtos/chrono.hpp"
#else
#  error "The OS wrapper has not been configured."
#endif

#endif // WEOS_CHRONO_HPP
