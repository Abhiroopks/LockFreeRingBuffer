#include <gtest/gtest.h>
#include "lfrb.hpp"

TEST(LockFreeRingBufferTest, Initialization) {
    LockFreeRingBuffer<int> buffer(5);
    SUCCEED();
}
