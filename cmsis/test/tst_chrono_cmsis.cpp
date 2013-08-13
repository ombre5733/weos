#include "../chrono.hpp"

#include "gtest/gtest.h"

TEST(Milliseconds, DefaultConstructor)
{
    osl::chrono::milliseconds ms;
    ASSERT_EQ(0, ms.count());
}

TEST(Milliseconds, ConstructorWithArgument)
{
    osl::chrono::milliseconds ms(42);
    ASSERT_EQ(42, ms.count());
}

TEST(Milliseconds, CopyConstructor)
{
    osl::chrono::milliseconds ms1(42);
    osl::chrono::milliseconds ms2(ms1);
    ASSERT_EQ(42, ms2.count());

}

TEST(Milliseconds, Zero)
{
    osl::chrono::milliseconds ms = osl::chrono::milliseconds::zero();
    ASSERT_EQ(0, ms.count());
}

TEST(Milliseconds, Min)
{
    osl::chrono::milliseconds ms = osl::chrono::milliseconds::min();
    ASSERT_EQ(-2147483648, ms.count());
}

TEST(Milliseconds, Max)
{
    osl::chrono::milliseconds ms = osl::chrono::milliseconds::max();
    ASSERT_EQ(2147483647, ms.count());
}
