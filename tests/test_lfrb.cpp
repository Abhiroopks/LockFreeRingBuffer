#include "lfrb.hpp"
#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>
#include <string>
#include <thread>

TEST(SingleThreadLFRBTest, Initialization)
{
    LockFreeRingBuffer<int> buffer(2);
    SUCCEED();
}

TEST(SingleThreadLFRBTest, EmptyPush)
{
    LockFreeRingBuffer<int> buffer(2);
    bool success = buffer.push(0);

    ASSERT_TRUE(success);
}

TEST(SingleThreadLFRBTest, EmptyPushMove)
{
    LockFreeRingBuffer<std::string> buffer(2);

    std::string elem = "Hello World! Testing Lock Free Ring Buffer.";
    bool success = buffer.push(std::move(elem));

    ASSERT_TRUE(success);
}

TEST(SingleThreadLFRBTest, FullPush)
{
    uint64_t capacity = 2;
    LockFreeRingBuffer<int> buffer(capacity);
    bool success;

    for (auto i = 0; i < capacity; i++) {
        success = buffer.push(std::move(i));
        ASSERT_TRUE(success);
    }

    success = buffer.push(capacity);
    ASSERT_FALSE(success);
}

TEST(SingleThreadLFRBTest, Pop)
{
    LockFreeRingBuffer<int> buffer(2);
    buffer.push(0);

    int dest;
    bool success = buffer.pop(dest);
    ASSERT_TRUE(success);

    ASSERT_EQ(0, dest);
}

TEST(SingleThreadLFRBTest, PopOrder)
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

TEST(SingleThreadLFRBTest, EmptyPop)
{
    LockFreeRingBuffer<int> buffer(2);

    int dest;
    bool success = buffer.pop(dest);
    ASSERT_FALSE(success);
}

TEST(MultiThreadLFRBTest, HighContentionCombo)
{
    const auto SIZE = (1 << 10);
    const auto NUM_THREADS = std::thread::hardware_concurrency();
    const auto NUM_PUSH_THREADS = NUM_THREADS / 2;
    const auto NUM_POP_THREADS = NUM_THREADS / 2;
    const auto TOTAL_ELEMS = SIZE;

    LockFreeRingBuffer<int> lfrb(SIZE);

    std::vector<std::thread> pushThreads;

    const auto elemsPerThread = TOTAL_ELEMS / (NUM_THREADS / 2);
    std::atomic<bool> startFlag{false};
    std::vector<bool> pushSuccess(TOTAL_ELEMS, false);

    for (auto i = 0; i < NUM_PUSH_THREADS; i++) {
        pushThreads.emplace_back([i, &pushSuccess, &lfrb, &startFlag, elemsPerThread]() {
            // wait for startFlag
            while (!startFlag.load()) {
                std::this_thread::yield();
            }

            int succIdx = i * elemsPerThread;

            for (auto j = 0; j < elemsPerThread; j++) {
                pushSuccess[succIdx] = lfrb.push(j);
                succIdx++;
            }
        });
    }

    std::vector<std::thread> popThreads;
    std::vector<bool> popSuccess(TOTAL_ELEMS, false);

    for (auto i = 0; i < NUM_POP_THREADS; i++) {
        popThreads.emplace_back([i, &popSuccess, &lfrb, &startFlag, elemsPerThread]() {
            // wait for startFlag
            while (!startFlag.load()) {
                std::this_thread::yield();
            }

            int val;
            int succIdx = i * elemsPerThread;
            int succCount = 0;

            while (!popSuccess[succIdx] && succCount < elemsPerThread) {
                if (lfrb.pop(val)) {
                    popSuccess[succIdx] = true;
                    succCount++;
                    succIdx++;
                }
            }
        });
    }

    startFlag.store(true);

    for (auto &t : pushThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
    for (auto &t : popThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    for (const auto &s : pushSuccess) {
        ASSERT_TRUE(s);
    }

    for (const auto &s : popSuccess) {
        ASSERT_TRUE(s);
    }
}

TEST(MultiThreadLFRBTest, HighContentionPop)
{
    const unsigned int numThreads = std::thread::hardware_concurrency();
    const unsigned int numElems = (1 << 13);
    const unsigned int elemsPerThread = numElems / numThreads;

    LockFreeRingBuffer<int> buffer(numElems);

    // fill the buffer first (single threaded)
    for (auto i = 0; i < numElems; i++) {
        buffer.push(i);
    }

    std::vector<std::thread> threads;
    std::vector<bool> success(numElems, false);
    std::atomic<bool> startFlag{false};

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([i, &startFlag, &buffer, &success, elemsPerThread]() {
            while (!startFlag.load()) {
                std::this_thread::yield();
            }

            int val;
            int succIdx = i * elemsPerThread;
            int succCount = 0;

            while (!success[succIdx] && succCount < elemsPerThread) {
                if (buffer.pop(val)) {
                    success[succIdx] = true;
                    succCount++;
                    succIdx++;
                }
            }
        });
    }

    startFlag.store(true);

    for (int i = 0; i < numThreads; i++) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }

    for (auto i = 0; i < numElems; i++) {
        ASSERT_TRUE(success[i]);
    }
}
TEST(MultiThreadLFRBTest, HighContentionPush)
{
    const int numThreads = 4;
    const int numElems = 4000;
    const int elemsPerThread = numElems / numThreads;

    LockFreeRingBuffer<int> buffer(numElems);
    std::vector<std::thread> threads;
    std::vector<bool> success(numElems, false);
    std::atomic<bool> startFlag{false};

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([i, &startFlag, &buffer, &success, elemsPerThread]() {
            while (!startFlag.load()) {
                std::this_thread::yield();
            }

            for (int j = 0; j < elemsPerThread; j++) {
                success[(i * elemsPerThread) + j] = buffer.push(int(i * j));
            }
        });
    }

    startFlag.store(true);

    for (int i = 0; i < numThreads; i++) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }

    for (auto i = 0; i < numElems; i++) {
        ASSERT_TRUE(success[i]);
    }
}

TEST(MultiThreadLFRBTest, EmptyPush)
{
    const int N = 4;
    LockFreeRingBuffer<int> buffer(N);
    std::array<std::thread, N> threads;
    std::array<bool, N> success;
    for (int i = 0; i < N; i++) {
        threads[i] = std::thread([i, &buffer, &success]() { success[i] = buffer.push(int(i)); });
    }

    for (int i = 0; i < N; i++) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }

    for (auto i = 0; i < N; i++) {
        ASSERT_TRUE(success[i]);
    }
}

TEST(MultiThreadLFRBTest, FullPush)
{
    const int N = 4;
    LockFreeRingBuffer<int> buffer(N / 2);
    std::array<std::thread, N> threads;
    std::array<bool, N> success;
    for (int i = 0; i < N; i++) {
        threads[i] = std::thread([i, &buffer, &success]() { success[i] = buffer.push(int(i)); });
    }

    for (int i = 0; i < N; i++) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }

    int fails = 0;
    for (auto i = 0; i < N; i++) {
        if (!success[i]) {
            fails++;
        }
    }

    ASSERT_EQ(fails, 2);
}

TEST(MultiThreadLFRBTest, Pop)
{
    const int N = 4;
    LockFreeRingBuffer<int> buffer(N);
    std::array<std::thread, N> pushThreads;
    std::array<bool, N> pushSuccess;
    for (int i = 0; i < N; i++) {
        pushThreads[i] = std::thread(
            [i, &buffer, &pushSuccess]() { pushSuccess[i] = buffer.push(int(i)); });
    }

    for (int i = 0; i < N; i++) {
        if (pushThreads[i].joinable()) {
            pushThreads[i].join();
        }
    }

    for (auto i = 0; i < N; i++) {
        ASSERT_TRUE(pushSuccess[i]);
    }

    std::array<std::thread, N> popThreads;
    std::array<bool, N> popSuccess;
    std::array<int, N> pops;

    for (int i = 0; i < N; i++) {
        popThreads[i] = std::thread(
            [i, &buffer, &popSuccess, &pops]() { popSuccess[i] = buffer.pop(pops[i]); });
    }

    for (int i = 0; i < N; i++) {
        if (popThreads[i].joinable()) {
            popThreads[i].join();
        }
    }

    for (auto i = 0; i < N; i++) {
        ASSERT_TRUE(popSuccess[i]);
    }

    std::sort(pops.begin(), pops.end());

    for (auto i = 0; i < N; i++) {
        ASSERT_EQ(pops[i], i);
    }
}

TEST(MultiThreadLFRBTest, PopFail)
{
    const int N = 4;
    const int numPushThreads = N / 2;

    LockFreeRingBuffer<int> buffer(N);
    std::array<std::thread, numPushThreads> pushThreads;
    std::array<bool, numPushThreads> pushSuccess;
    for (int i = 0; i < numPushThreads; i++) {
        pushThreads[i] = std::thread(
            [i, &buffer, &pushSuccess]() { pushSuccess[i] = buffer.push(int(i)); });
    }

    for (int i = 0; i < numPushThreads; i++) {
        if (pushThreads[i].joinable()) {
            pushThreads[i].join();
        }
    }

    for (auto i = 0; i < numPushThreads; i++) {
        ASSERT_TRUE(pushSuccess[i]);
    }

    std::array<std::thread, N> popThreads;
    std::array<bool, N> popSuccess;
    std::array<int, N> pops;

    // this is just a large number indicating a junk value.
    pops.fill(N * 2);

    for (int i = 0; i < N; i++) {
        popThreads[i] = std::thread(
            [i, &buffer, &popSuccess, &pops]() { popSuccess[i] = buffer.pop(pops[i]); });
    }

    for (int i = 0; i < N; i++) {
        if (popThreads[i].joinable()) {
            popThreads[i].join();
        }
    }

    int popFails = 0;
    for (auto i = 0; i < N; i++) {
        if (!popSuccess[i]) {
            popFails++;
        }
    }

    ASSERT_EQ(popFails, N - numPushThreads);

    std::sort(pops.begin(), pops.end());

    // ensure the items pushed are actually popped.
    for (auto i = 0; i < numPushThreads; i++) {
        ASSERT_EQ(pops[i], i);
    }
}
