#include "lfrb.hpp"
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>


// ─── Benchmark definition ──────────────────────────────────────────────────────

struct Benchmark
{
    std::string name;
    std::function<void()> run; // each benchmark prints its own results
};

// ─── Individual benchmarks ─────────────────────────────────────────────────────

void bench_dequeue()
{
    const auto SIZE = (1 << 13);
    const auto NUM_THREADS = std::thread::hardware_concurrency();
    const auto TOTAL_ELEMS = SIZE;

    LockFreeRingBuffer<std::string> lfrb(SIZE);

    // Fill the buffer first (single threaded)
    for (auto i = 0; i < TOTAL_ELEMS; i++) {
        lfrb.push(std::to_string(i));
    }

    // Multithreaded pops.
    std::vector<std::thread> threads;
    const auto elemsPerThread = TOTAL_ELEMS / NUM_THREADS;
    std::atomic<bool> startFlag{false};

    for (auto i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([i, &lfrb, &startFlag, elemsPerThread]() {
            // wait for startFlag
            while (!startFlag.load()) {
                std::this_thread::yield();
            }

            std::string val;

            for (auto j = 0; j < elemsPerThread; j++) {
                lfrb.pop(val);
            }
        });
    }

    startFlag.store(true);

    auto start = std::chrono::steady_clock::now();
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;

    double elapsedSeconds = std::chrono::duration<double>(diff).count();
    double elemsPerSecond = static_cast<double>(TOTAL_ELEMS) / elapsedSeconds;

    std::cout << "Popped " << TOTAL_ELEMS << " elements in "
              << std::chrono::duration_cast<std::chrono::microseconds>(diff) << " ("
              << elemsPerSecond << " elems/s) with " << NUM_THREADS << " threads\n";
}

void bench_enqueue()
{
    const auto SIZE = (1 << 13);
    const auto NUM_THREADS = std::thread::hardware_concurrency();
    const auto TOTAL_ELEMS = SIZE;

    LockFreeRingBuffer<std::string> lfrb(SIZE);

    std::vector<std::thread> threads;

    const auto elemsPerThread = TOTAL_ELEMS / NUM_THREADS;
    std::atomic<bool> startFlag{false};

    for (auto i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([i, &lfrb, &startFlag, elemsPerThread]() {
            // wait for startFlag
            while (!startFlag.load()) {
                std::this_thread::yield();
            }

            for (auto j = 0; j < elemsPerThread; j++) {
                lfrb.push("test msg");
            }
        });
    }

    startFlag.store(true);

    auto start = std::chrono::steady_clock::now();
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;

    double elapsedSeconds = std::chrono::duration<double>(diff).count();
    double elemsPerSecond = static_cast<double>(TOTAL_ELEMS) / elapsedSeconds;

    std::cout << "Pushed " << TOTAL_ELEMS << " elements in "
              << std::chrono::duration_cast<std::chrono::microseconds>(diff) << " ("
              << elemsPerSecond << " elems/s) with " << NUM_THREADS << " threads\n";
}

// ─── Main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::vector<Benchmark> benchmarks = {{"Enqueue", bench_enqueue}, {"Dequeue", bench_dequeue}};

    for (auto &b : benchmarks) {
        std::cout << "\n=== " << b.name << " ===\n";
        b.run();
    }

    return 0;
}
