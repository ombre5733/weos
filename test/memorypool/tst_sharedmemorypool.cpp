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
class SharedMemoryPoolTestFixture : public testing::Test
{
};

// Define a list of types with which the memory pool will be instantiated.
typedef testing::Types<
    std::int8_t,  std::int16_t,  std::int32_t,  std::int64_t,  std::intptr_t,  std::intmax_t,
    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t, std::uintptr_t, std::uintmax_t,
    float, double, long double> TypesToTest;
TYPED_TEST_CASE(SharedMemoryPoolTestFixture, TypesToTest);

TYPED_TEST(SharedMemoryPoolTestFixture, Constructor)
{
    {
        weos::shared_memory_pool<TypeParam, 1> p;
        ASSERT_FALSE(p.empty());
        ASSERT_EQ(1, p.capacity());
        //ASSERT_EQ(1, p.size());
    }

    {
        weos::shared_memory_pool<TypeParam, 10> p;
        ASSERT_FALSE(p.empty());
        ASSERT_EQ(10, p.capacity());
        //ASSERT_EQ(10, p.size());
    }
}

TYPED_TEST(SharedMemoryPoolTestFixture, allocate)
{
    const unsigned POOL_SIZE = 10;
    weos::shared_memory_pool<TypeParam, POOL_SIZE> p;
    char* chunks[POOL_SIZE];

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        //ASSERT_EQ(POOL_SIZE - i, p.size());
        ASSERT_FALSE(p.empty());
        void* c = p.try_allocate();
        ASSERT_TRUE(c != 0);
        //ASSERT_EQ(POOL_SIZE - i - 1, p.size());

        // Check the alignment of the allocated chunk.
        char* addr = static_cast<char*>(c);
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(addr)
                    % weos::alignment_of<TypeParam>::value == 0);

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
    ASSERT_EQ(POOL_SIZE, p.capacity());
}

TYPED_TEST(SharedMemoryPoolTestFixture, try_allocate)
{
    const unsigned POOL_SIZE = 10;
    weos::shared_memory_pool<TypeParam, POOL_SIZE> p;

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        void* c = p.try_allocate();
        ASSERT_TRUE(c != 0);
    }
    ASSERT_TRUE(p.empty());
    ASSERT_EQ(POOL_SIZE, p.capacity());

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        ASSERT_TRUE(p.try_allocate() == 0);
    }
}

TYPED_TEST(SharedMemoryPoolTestFixture, allocate_and_free)
{
    const unsigned POOL_SIZE = 10;
    weos::shared_memory_pool<TypeParam, POOL_SIZE> p;
    void* chunks[POOL_SIZE];

    for (unsigned j = 1; j <= POOL_SIZE; ++j)
    {
        for (unsigned i = 0; i < j; ++i)
        {
            //ASSERT_EQ(POOL_SIZE - i, p.size());
            void* c = p.try_allocate();
            ASSERT_TRUE(c != 0);
            //ASSERT_EQ(POOL_SIZE - i - 1, p.size());
            ASSERT_EQ(POOL_SIZE, p.capacity());
            chunks[i] = c;
        }

        for (unsigned i = 0; i < j; ++i)
        {
            //ASSERT_EQ(POOL_SIZE - j + i, p.size());
            p.free(chunks[i]);
            //ASSERT_EQ(POOL_SIZE - j + i + 1, p.size());
        }
    }
}

TYPED_TEST(SharedMemoryPoolTestFixture, random_allocate_and_free)
{
    const unsigned POOL_SIZE = 10;
    weos::shared_memory_pool<TypeParam, POOL_SIZE> p;
    void* chunks[POOL_SIZE];
    std::set<void*> uniqueChunks;
    int numAllocatedChunks = 0;

    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        void* c = p.try_allocate();
        ASSERT_TRUE(c != 0);
        chunks[i] = c;
        uniqueChunks.insert(c);
    }
    ASSERT_TRUE(p.empty());
    //ASSERT_EQ(POOL_SIZE, uniqueChunks.size());
    for (unsigned i = 0; i < POOL_SIZE; ++i)
    {
        p.free(chunks[i]);
        chunks[i] = 0;
    }

    for (unsigned i = 0; i < 10000; ++i)
    {
        unsigned index = testing::random() % POOL_SIZE;
        if (chunks[index] == 0)
        {
            void* c = p.try_allocate();
            ASSERT_TRUE(c != 0);
            ASSERT_TRUE(uniqueChunks.find(c) != uniqueChunks.end());
            chunks[index] = c;
            ++numAllocatedChunks;
        }
        else
        {
            p.free(chunks[index]);
            chunks[index] = 0;
            --numAllocatedChunks;
        }
        //ASSERT_EQ(POOL_SIZE - numAllocatedChunks, p.size());
        ASSERT_EQ(POOL_SIZE, p.capacity());
    }
}
