# LockFreeRingBuffer
A Lock-Free Ring Buffer implementation in C++.

## Build & Test Instructions

```bash

# Configure
cmake -B build -S .

# Build
cmake --build build

# Run Tests
ctest --test-dir build --output-on-failure
```

## The Protocol

Each slot has two fields: the payload and an std::atomic<int64_t> sequence.

slot: { T data; std::atomic<int64_t> seq; }


### Producer (push):

```cpp
bool push(T val) {
    int64_t pos = tail.load(std::memory_order_relaxed);
    for (;;) {
        Slot& s = slots[pos & MASK];
        int64_t seq = s.seq.load(std::memory_order_acquire);
        int64_t diff = seq - pos;

        if (diff == 0) {
            // Slot is ready for this position — try to claim it
            if (tail.compare_exchange_weak(pos, pos + 1,
                    std::memory_order_acq_rel))
                break;  // We exclusively own this slot now
        } else if (diff < 0) {
            return false;  // Buffer full
        }
        // diff > 0: another producer claimed it but hasn't published yet; spin
    }

    // Plain (non-atomic) write — only this thread touches this slot
    s.data = val;

    // Publish: release ensures the data write is visible before the seq update
    s.seq.store(pos + 1, std::memory_order_release);
    return true;
}
```



### Consumer (pop):

```cpp
bool pop(T& out) {
    int64_t pos = head.load(std::memory_order_relaxed);
    for (;;) {
        Slot& s = slots[pos & MASK];
        int64_t seq = s.seq.load(std::memory_order_acquire);
        int64_t diff = seq - (pos + 1);

        if (diff == 0) {
            // Data is published — try to claim the read
            if (head.compare_exchange_weak(pos, pos + 1,
                    std::memory_order_acq_rel)) {
                out = s.data;  // Plain read — safe, we own it
                s.seq.store(pos + CAPACITY, std::memory_order_release);
                return true;
            }
        } else if (diff < 0) {
            return false;  // Buffer empty
        }
    }
}
```
