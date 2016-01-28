/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2016, Manuel Freiberger
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

// Define a list of types with which the memory pool will be instantiated.
typedef testing::Types<
    std::int8_t,  std::int16_t,  std::int32_t,  std::int64_t,  std::intptr_t,  std::intmax_t,
    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t, std::uintptr_t, std::uintmax_t,
    float, double, long double> TypesToTest;
TYPED_TEST_CASE(MemoryPoolTestFixture, TypesToTest);

TYPED_TEST(MemoryPoolTestFixture, Constructor)
{
    {
        weos::memory_pool<TypeParam, 1> p;
        ASSERT_FALSE(p.empty());
        ASSERT_EQ(1, p.capacity());
    }

    {
        weos::memory_pool<TypeParam, 10> p;
        ASSERT_FALSE(p.empty());
        ASSERT_EQ(10, p.capacity());
    }
}

TYPED_TEST(MemoryPoolTestFixture, try_allocate)
{
    const unsigned POOL_SIZE = 10;
    weos::memory_pool<TypeParam, POOL_SIZE> p;
    char* chunks[POOL_SIZE];

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        ASSERT_FALSE(p.empty());
        void* c = p.try_allocate();
        ASSERT_TRUE(c != 0);

        // Check the alignment of the allocated chunk.
        char* addr = static_cast<char*>(c);
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(addr)
                    % weos::alignment_of<TypeParam>::value == 0);

        // Attempt to store and load something in the chunk. Some processors
        // might throw a hard-fault if an incorrect memory access occurs.
        {
            volatile TypeParam* chunk = static_cast<TypeParam*>(c);
            *chunk = i;
            TypeParam dummy = *chunk;
            ASSERT_EQ(i, dummy);
        }

        // Make sure that no chunk is returned twice from the pool and that
        // the chunks do not overlap.
        for (unsigned j = 0; j < i; ++j)
        {
            ASSERT_FALSE(chunks[j] == addr);

            if (chunks[j] < addr)
                ASSERT_TRUE(chunks[j] + sizeof(TypeParam) <= addr);
            if (chunks[j] > addr)
                ASSERT_TRUE(addr + sizeof(TypeParam) <= chunks[j]);
        }
        chunks[i] = addr;
    }

    ASSERT_TRUE(p.empty());
    ASSERT_EQ(POOL_SIZE, p.capacity());
}

TYPED_TEST(MemoryPoolTestFixture, allocate_and_free)
{
    const unsigned POOL_SIZE = 10;
    weos::memory_pool<TypeParam, POOL_SIZE> p;
    void* chunks[POOL_SIZE];

    for (unsigned j = 1; j <= 2 * POOL_SIZE; ++j)
    {
        for (unsigned i = 0; i < std::min(j, POOL_SIZE); ++i)
        {
            void* c = p.try_allocate();
            ASSERT_TRUE(c != 0);
            chunks[i] = c;
        }

        ASSERT_EQ(POOL_SIZE, p.capacity());

        // Allocating from an empty pool returns a null-pointer.
        for (unsigned i = POOL_SIZE; i < j; ++i)
        {
            ASSERT_TRUE(p.empty());
            ASSERT_TRUE(p.try_allocate() == 0);
        }

        for (unsigned i = 0; i < std::min(j, POOL_SIZE); ++i)
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

    // Allocate all chunks at least once so that we can create a set of
    // chunk pointers.
    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        void* c = p.try_allocate();
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

    // Now allocate and free the chunks in random order.
    for (unsigned i = 0; i < 10000; ++i)
    {
        unsigned index = testing::random() % POOL_SIZE;
        if (chunks[index] == 0)
        {
            void* c = p.try_allocate();
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
        void* c = s.p.try_allocate();
        ASSERT_TRUE(c != 0);

        // Check the alignment of the allocated chunk.
        char* addr = static_cast<char*>(c);
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(addr)
                    % weos::alignment_of<TypeParam>::value == 0);

        // Attempt to store and load something in the chunk. Some processors
        // might throw a hard-fault if an incorrect memory access occurs.
        {
            volatile TypeParam* chunk = static_cast<TypeParam*>(c);
            *chunk = i;
            TypeParam dummy = *chunk;
            ASSERT_EQ(i, dummy);
        }

        // Make sure that no chunk is returned twice from the pool and that
        // the chunks do not overlap.
        for (unsigned j = 0; j < i; ++j)
        {
            ASSERT_FALSE(chunks[j] == addr);

            if (chunks[j] < addr)
                ASSERT_TRUE(chunks[j] + sizeof(TypeParam) <= addr);
            if (chunks[j] > addr)
                ASSERT_TRUE(addr + sizeof(TypeParam) <= chunks[j]);
        }
        chunks[i] = addr;
    }

    ASSERT_TRUE(s.p.empty());
    ASSERT_EQ(POOL_SIZE, s.p.capacity());
}
