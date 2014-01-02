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

#include <memorypool.hpp>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

#include <set>

template <typename T>
class MemoryPoolTestFixture : public testing::Test
{
};

typedef testing::Types<
    std::int8_t, std::int16_t, std::int32_t, std::int64_t,
    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
    float, double, long double> TypesToTest;
TYPED_TEST_CASE(MemoryPoolTestFixture, TypesToTest);

TYPED_TEST(MemoryPoolTestFixture, Constructor)
{
    weos::memory_pool<TypeParam, 10> p;
    ASSERT_FALSE(p.empty());
    ASSERT_EQ(10, p.capacity());
}

TYPED_TEST(MemoryPoolTestFixture, allocate)
{
    const unsigned POOL_SIZE = 10;
    weos::memory_pool<TypeParam, POOL_SIZE> p;
    char* chunks[POOL_SIZE];

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        ASSERT_FALSE(p.empty());
        void* c = p.allocate();
        ASSERT_TRUE(c != 0);

        // Check the alignment of the allocated chunk.
        char* addr = static_cast<char*>(c);
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(addr)
                    % boost::alignment_of<TypeParam>::value == 0);

        for (unsigned j = 0; j < i; ++j)
        {
            // No chunk can be returned twice from the pool.
            ASSERT_FALSE(chunks[j] == addr);

            // Chunks must not overlap.
            if (chunks[j] < addr)
                ASSERT_TRUE(chunks[j] + sizeof(TypeParam) <= addr);
            if (chunks[j] > addr)
                ASSERT_TRUE(addr + sizeof(TypeParam) <= chunks[j]);
        }
        chunks[i] = addr;
    }

    ASSERT_TRUE(p.empty());
}

TYPED_TEST(MemoryPoolTestFixture, allocate_and_free)
{
    const unsigned POOL_SIZE = 10;
    weos::memory_pool<TypeParam, POOL_SIZE> p;
    void* chunks[POOL_SIZE];

    for (unsigned j = 1; j <= POOL_SIZE; ++j)
    {
        for (unsigned i = 0; i < j; ++i)
        {
            void* c = p.allocate();
            ASSERT_TRUE(c != 0);
            chunks[i] = c;
        }
        for (unsigned i = 0; i < j; ++i)
        {
            p.free(chunks[i]);
        }
    }
}

TYPED_TEST(MemoryPoolTestFixture, random_allocate_and_free)
{
    const unsigned POOL_SIZE = 10;
    weos::memory_pool<TypeParam, POOL_SIZE> p;
    void* chunks[POOL_SIZE];
    std::set<void*> uniqueChunks;

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        void* c = p.allocate();
        ASSERT_TRUE(c != 0);
        chunks[i] = c;
        uniqueChunks.insert(c);
    }
    ASSERT_TRUE(p.empty());
    ASSERT_EQ(POOL_SIZE, uniqueChunks.size());
    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        p.free(chunks[i]);
        chunks[i] = 0;
    }

    for (unsigned i = 0; i < 2000; ++i)
    {
        unsigned index = random() % POOL_SIZE;
        if (chunks[index] == 0)
        {
            void* c = p.allocate();
            ASSERT_TRUE(c != 0);
            ASSERT_TRUE(uniqueChunks.find(c) != uniqueChunks.end());
            chunks[index] = c;
        }
        else
        {
            p.free(chunks[index]);
            chunks[index] = 0;
        }
    }
}

template <typename TType, std::size_t TSize>
struct test_struct
{
    char dummy;
    weos::memory_pool<TType, TSize> p;
};

TYPED_TEST(MemoryPoolTestFixture, allocate_inside_struct)
{
    const unsigned POOL_SIZE = 10;
    test_struct<TypeParam, POOL_SIZE> s;
    char* chunks[POOL_SIZE];

    ASSERT_TRUE(static_cast<void*>(&s.dummy) != static_cast<void*>(&s.p));

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        ASSERT_FALSE(s.p.empty());
        void* c = s.p.allocate();
        ASSERT_TRUE(c != 0);

        // Check the alignment of the allocated chunk.
        char* addr = static_cast<char*>(c);
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(addr)
                    % boost::alignment_of<TypeParam>::value == 0);

        for (unsigned j = 0; j < i; ++j)
        {
            // No chunk can be returned twice from the pool.
            ASSERT_FALSE(chunks[j] == addr);

            // Chunks must not overlap.
            if (chunks[j] < addr)
                ASSERT_TRUE(chunks[j] + sizeof(TypeParam) <= addr);
            if (chunks[j] > addr)
                ASSERT_TRUE(addr + sizeof(TypeParam) <= chunks[j]);
        }
        chunks[i] = addr;
    }

    ASSERT_TRUE(s.p.empty());
}


#include "stm32f4xx.h"
#include <boost/move/move.hpp>
#include <boost/type_traits.hpp>
#include "thread.hpp"




#if 0
namespace weos
{
namespace detail
{

class MyThreadDataBase
{
public:
    virtual ~MyThreadDataBase() {}

    virtual void run() = 0;
};

// The largest possible MyThreadData object.
struct LargestMyThreadData : public MyThreadDataBase
{
    virtual void run() {}
    boost::aligned_storage<4*32>::type m_data;
};

struct null_type {};

template <typename F,
          typename A0 = null_type, typename A1 = null_type,
          typename A2 = null_type, typename A3 = null_type>
struct MyThreadData : public MyThreadDataBase
{

    MyThreadData(F f,
                 BOOST_RV_REF(A0) a0,
                 BOOST_RV_REF(A1) a1,
                 BOOST_RV_REF(A2) a2,
                 BOOST_RV_REF(A3) a3)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1)),
          m_a2(boost::forward<A2>(a2)),
          m_a3(boost::forward<A3>(a3))
    {
        BOOST_STATIC_ASSERT(
            sizeof(MyThreadData) <= sizeof(LargestMyThreadData)
            && boost::alignment_of<MyThreadData>::value
               <= boost::alignment_of<LargestMyThreadData>::value);
    }

    virtual void run()
    {
        invokeCallable(m_f,
                       boost::move(m_a0),
                       boost::move(m_a1),
                       boost::move(m_a2),
                       boost::move(m_a3));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
    typename boost::decay<A2>::type m_a2;
    typename boost::decay<A3>::type m_a3;
};

template <typename F, typename A0, typename A1, typename A2>
struct MyThreadData<F, A0, A1, A2, null_type>
        : public MyThreadDataBase
{
    MyThreadData(F f,
                 BOOST_RV_REF(A0) a0,
                 BOOST_RV_REF(A1) a1,
                 BOOST_RV_REF(A2) a2)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1)),
          m_a2(boost::forward<A2>(a2))
    {
        BOOST_STATIC_ASSERT(
            sizeof(MyThreadData) <= sizeof(LargestMyThreadData)
            && boost::alignment_of<MyThreadData>::value
               <= boost::alignment_of<LargestMyThreadData>::value);
    }

    virtual void run()
    {
        invokeCallable(m_f,
                       boost::move(m_a0),
                       boost::move(m_a1),
                       boost::move(m_a2));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
    typename boost::decay<A2>::type m_a2;
};

template <typename F, typename A0, typename A1>
struct MyThreadData<F, A0, A1, null_type, null_type>
        : public MyThreadDataBase
{
    MyThreadData(F f,
                 BOOST_RV_REF(A0) a0,
                 BOOST_RV_REF(A1) a1)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1))
    {
        BOOST_STATIC_ASSERT(
            sizeof(MyThreadData) <= sizeof(LargestMyThreadData)
            && boost::alignment_of<MyThreadData>::value
               <= boost::alignment_of<LargestMyThreadData>::value);
    }

    virtual void run()
    {
        invokeCallable(m_f,
                       boost::move(m_a0),
                       boost::move(m_a1));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
};

template <typename F, typename A0>
struct MyThreadData<F, A0, null_type, null_type, null_type>
        : public MyThreadDataBase
{
    MyThreadData(F f,
                 BOOST_RV_REF(A0) a0)
        : m_f(f),
          m_a0(boost::forward<A0>(a0))
    {
        BOOST_STATIC_ASSERT(
            sizeof(MyThreadData) <= sizeof(LargestMyThreadData)
            && boost::alignment_of<MyThreadData>::value
               <= boost::alignment_of<LargestMyThreadData>::value);
    }

    virtual void run()
    {
        invokeCallable(m_f,
                       boost::move(m_a0));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
};

template <typename F>
struct MyThreadData<F, null_type, null_type, null_type, null_type>
        : public MyThreadDataBase
{
    explicit MyThreadData(F f)
        : m_f(f)
    {
        BOOST_STATIC_ASSERT(
            sizeof(MyThreadData) <= sizeof(LargestMyThreadData)
            && boost::alignment_of<MyThreadData>::value
               <= boost::alignment_of<LargestMyThreadData>::value);
    }

    virtual void run()
    {
        m_f();
    }

    typename boost::decay<F>::type m_f;
};


} // namespace detail
} // namespace weos

namespace weos
{

struct MyThread
{
public:
    template <typename F>
    explicit MyThread(F f)
        : m_data(allocateMyThreadData())
    {
        new (m_data) detail::MyThreadData<
                typename boost::remove_reference<F>::type>(f);
    }

    template <typename F, typename A0>
    MyThread(F f,
             BOOST_RV_REF(A0) a0)
        : m_data(allocateMyThreadData())
    {
        new (m_data) detail::MyThreadData<
                typename boost::remove_reference<F>::type,
                A0>(
                    f,
                    boost::forward<A0>(a0));
    }

    template <typename F, typename A0, typename A1>
    MyThread(F f,
             BOOST_RV_REF(A0) a0,
             BOOST_RV_REF(A1) a1)
        : m_data(allocateMyThreadData())
    {
        new (m_data) detail::MyThreadData<
                typename boost::remove_reference<F>::type,
                A0, A1>(
                    f,
                    boost::forward<A0>(a0),
                    boost::forward<A1>(a1));
    }

    template <typename F, typename A0, typename A1, typename A2>
    MyThread(F f,
             BOOST_RV_REF(A0) a0,
             BOOST_RV_REF(A1) a1,
             BOOST_RV_REF(A2) a2)
        : m_data(allocateMyThreadData())
    {
        new (m_data) detail::MyThreadData<
                typename boost::remove_reference<F>::type,
                A0, A1, A2>(
                    f,
                    boost::forward<A0>(a0),
                    boost::forward<A1>(a1),
                    boost::forward<A2>(a2));
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2,
              typename A3>
    MyThread(F f,
             BOOST_RV_REF(A0) a0,
             BOOST_RV_REF(A1) a1,
             BOOST_RV_REF(A2) a2,
             BOOST_RV_REF(A3) a3)
        : m_data(allocateMyThreadData())
    {
        new (m_data) detail::MyThreadData<
                typename boost::remove_reference<F>::type,
                A0, A1, A2, A3>(
                    f,
                    boost::forward<A0>(a0),
                    boost::forward<A1>(a1),
                    boost::forward<A2>(a2),
                    boost::forward<A3>(a3));
    }

private:
    static detail::MyThreadDataBase* allocateMyThreadData()
    {
        static memory_pool<detail::LargestMyThreadData,
                           WEOS_MAX_NUM_CONCURRENT_THREADS,
                           mutex> pool;

        void* mem = pool.allocate();
        if (!mem)
            ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value

        return static_cast<detail::MyThreadDataBase*>(mem);
    }

public:
    detail::MyThreadDataBase* m_data;
};

} // namespace weos
#endif

void f0()
{
    GPIOD->ODR ^= 1 << 13;
}

void f1(int a)
{
    if (a == 0)
        GPIOD->BRR = 1 << 12;
    else
        GPIOD->BSR = 1 << 12;
}

void f2(int a, int b)
{
    if (a == 0)
        GPIOD->BRR = 1 << 12;
    else
        GPIOD->BSR = 1 << 12;

    if (b == 0)
        GPIOD->BRR = 1 << 13;
    else
        GPIOD->BSR = 1 << 13;
}

void f3(int a, int b, int c)
{
    if (a == 0)
        GPIOD->BRR = 1 << 12;
    else
        GPIOD->BSR = 1 << 12;

    if (b == 0)
        GPIOD->BRR = 1 << 13;
    else
        GPIOD->BSR = 1 << 13;

    if (c == 0)
        GPIOD->BRR = 1 << 14;
    else
        GPIOD->BSR = 1 << 14;
}

void f4(int a, int b, int c, int d)
{
    if (a == 0)
        GPIOD->BRR = 1 << 12;
    else
        GPIOD->BSR = 1 << 12;

    if (b == 0)
        GPIOD->BRR = 1 << 13;
    else
        GPIOD->BSR = 1 << 13;

    if (c == 0)
        GPIOD->BRR = 1 << 14;
    else
        GPIOD->BSR = 1 << 14;

    if (d == 0)
        GPIOD->BRR = 1 << 15;
    else
        GPIOD->BSR = 1 << 15;
}

struct MemberFun0
{
    void clearLed()
    {
        GPIOD->BRR = 1 << 12;
    }

    void setLed()
    {
        GPIOD->BSR = 1 << 12;
    }
};

struct MemberFun1
{
    void setLed(int a)
    {
        if (a == 0)
            GPIOD->BRR = 1 << 12;
        else
            GPIOD->BSR = 1 << 12;
    }
};

struct MemberFun2
{
    void setLed(int a, int b)
    {
        if (a == 0)
            GPIOD->BRR = 1 << 12;
        else
            GPIOD->BSR = 1 << 12;

        if (b == 0)
            GPIOD->BRR = 1 << 13;
        else
            GPIOD->BSR = 1 << 13;
    }
};

struct MemberFun3
{
    void setLed(int a, int b, int c)
    {
        if (a == 0)
            GPIOD->BRR = 1 << 12;
        else
            GPIOD->BSR = 1 << 12;

        if (b == 0)
            GPIOD->BRR = 1 << 13;
        else
            GPIOD->BSR = 1 << 13;

        if (c == 0)
            GPIOD->BRR = 1 << 14;
        else
            GPIOD->BSR = 1 << 14;
    }
};

struct MovableMemberFun0
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(MovableMemberFun0)

public:
    MovableMemberFun0(int* someData)
        : m_someData(someData)
    {
    }

    // Move constructor.
    MovableMemberFun0(BOOST_RV_REF(MovableMemberFun0) other)
        : m_someData(other.m_someData)
    {
        other.m_someData = 0;
    }

    void toggleLed()
    {
        GPIOD->ODR ^= 1 << 12;
    }

private:
    int* m_someData;
};

/*
TEST(XX, yy) // --> works
{
    MovableMemberFun0 m(0);

    weos::thread mt1(&MovableMemberFun0::toggleLed, boost::move(m));

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    MemberFun3 m;

    weos::thread mt1(&MemberFun3::setLed, &m, 1, 0, 0);
    weos::thread mt2(&MemberFun3::setLed, &m, 0, 1, 0);
    weos::thread mt3(&MemberFun3::setLed, &m, 0, 0, 1);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
        mt3.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    MemberFun2 m;

    weos::thread mt1(&MemberFun2::setLed, &m, 1, 0);
    weos::thread mt2(&MemberFun2::setLed, &m, 0, 1);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    MemberFun1 m;

    weos::thread mt1(&MemberFun1::setLed, &m, 0);
    weos::thread mt2(&MemberFun1::setLed, &m, 1);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    MemberFun0 c;

    weos::thread mt1(&MemberFun0::clearLed, c);
    weos::thread mt2(&MemberFun0::setLed, c);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    MemberFun0 c;

    weos::thread mt1(&MemberFun0::clearLed, &c);
    weos::thread mt2(&MemberFun0::setLed, &c);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    weos::thread mt1(&f4, 1, 0, 0, 0);
    weos::thread mt2(&f4, 0, 1, 0, 0);
    weos::thread mt3(&f4, 0, 0, 1, 0);
    weos::thread mt4(&f4, 0, 0, 0, 1);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
        mt3.m_data->run();
        osDelay(100);
        mt4.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    weos::thread mt1(&f3, 1, 0, 0);
    weos::thread mt2(&f3, 0, 1, 0);
    weos::thread mt3(&f3, 0, 0, 1);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
        mt3.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    weos::thread mt1(&f2, 1, 0);
    weos::thread mt2(&f2, 0, 1);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
    }
}
*/

/*
TEST(XX, yy) // --> works
{
    weos::thread mt1(&f1, 0);
    weos::thread mt2(&f1, 1);

    for (int i = 0; i < 20; ++i)
    {
        mt1.m_data->run();
        osDelay(100);
        mt2.m_data->run();
        osDelay(100);
    }
}
*/

TEST(XX, yy) // --> works
{
    for (int i = 0; i < 20; ++i)
    {
        weos::thread mt1(&f0);
        mt1.join();
        osDelay(100);
    }
}
