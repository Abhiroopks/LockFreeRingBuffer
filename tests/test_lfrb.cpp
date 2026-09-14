#include <gtest/gtest.h>
#include "lfrb.hpp"

TEST(SingleThreadLockFreeRingBufferTest, BufferInitialization)
{
    LockFreeRingBuffer<int> buffer(2);
    SUCCEED();
}

TEST(SingleThreadLockFreeRingBufferTest, EmptyBufferPush)
{
    LockFreeRingBuffer<int> buffer(2);
    bool success = buffer.push(0);

    ASSERT_TRUE(success);
}

TEST(SingleThreadLockFreeRingBufferTest, FullBufferPush)
{
    int64_t capacity = 2;
    LockFreeRingBuffer<int> buffer(capacity);
    bool success;

    for (auto i = 0; i < capacity; i++) {
        success = buffer.push(std::move(i));
        ASSERT_TRUE(success);
    }

    success = buffer.push(capacity);
    ASSERT_FALSE(success);
}

TEST(SingleThreadLockFreeRingBufferTest, BufferPop)
{
    LockFreeRingBuffer<int> buffer(2);
    buffer.push(0);

    int dest;
    bool success = buffer.pop(dest);
    ASSERT_TRUE(success);

    ASSERT_EQ(0, dest);
}

TEST(SingleThreadLockFreeRingBufferTest, BufferPopOrder)
{
    LockFreeRingBuffer<int> buffer(2);
    buffer.push(0);
    buffer.push(1);

    int dest;
    bool success = buffer.pop(dest);

    ASSERT_TRUE(success);
    ASSERT_EQ(0, dest);

    success = buffer.pop(dest);
    ASSERT_TRUE(success);
    ASSERT_EQ(1, dest);
}

TEST(SingleThreadLockFreeRingBufferTest, EmptyBufferPop)
{
    LockFreeRingBuffer<int> buffer(2);

    int dest;
    bool success = buffer.pop(dest);
    ASSERT_FALSE(success);
}
