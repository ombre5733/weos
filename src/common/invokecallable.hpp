/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef WEOS_COMMON_INOVKECALLABLE_HPP
#define WEOS_COMMON_INOVKECALLABLE_HPP

#include "../config.hpp"

#include <boost/move/move.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace weos
{
namespace detail
{

// ----=====================================================================----
//     invokeCallable
// ----=====================================================================----

// --------------------------------------------------------------------
//     Case 1: Member function pointer together with object
// --------------------------------------------------------------------
template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(),
               BOOST_FWD_REF(A0) a0)
{
    (boost::forward<A0>(a0).*f)(
        );
}

template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)() const,
               BOOST_FWD_REF(A0) a0)
{
    (boost::forward<A0>(a0).*f)(
        );
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

// --------------------------------------------------------------------
//     Case 2: Member function pointer together with pointer
// --------------------------------------------------------------------
template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(),
               BOOST_FWD_REF(A0) a0)
{
    ((*boost::forward<A0>(a0)).*f)(
        );
}

template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)() const,
               BOOST_FWD_REF(A0) a0)
{
    ((*boost::forward<A0>(a0)).*f)(
        );
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

// --------------------------------------------------------------------
//     Case 3: Function pointer
// --------------------------------------------------------------------
template <typename F>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f)
{
    boost::forward<F>(f)(
        );
}

template <typename F,
          typename A0>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0));
}

template <typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0),
        boost::forward<A1>(a1));
}

template <typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0),
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0),
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

} // namespace detail
} // namespace weos

#endif // WEOS_COMMON_INOVKECALLABLE_HPP
