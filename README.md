# LockFreeRingBuffer

A header-only, lock-free Multi-Producer Multi-Consumer (MPMC) bounded ring buffer implemented in C++20. Based on Dmitry Vyukov's MPMC design.

## Overview

`LockFreeRingBuffer` is a bounded FIFO queue designed for concurrent systems. Multiple producers and consumers can push and pop elements concurrently without mutexes or kernel locks.

- **Lock-Free MPMC:** Safe for concurrent access across multiple producer and consumer threads.
- **Power-of-2 Sizing:** Requested buffer capacity is automatically rounded up to the nearest power of 2 for fast indexing.
- **Header-Only:** Single header file (`lfrb.hpp`), zero external dependencies.
- **Requirements:** C++20 compliant compiler, CMake 3.20+.

## Integration (CMake)

The library provides an `INTERFACE` CMake target `LockFreeRingBuffer::LockFreeRingBuffer`.

### Using `FetchContent` (Recommended)

```cmake
include(FetchContent)

FetchContent_Declare(
    LockFreeRingBuffer
    GIT_REPOSITORY https://github.com/Abhiroopks/LockFreeRingBuffer.git
    GIT_TAG        main # or a specific tag/commit
)
FetchContent_MakeAvailable(LockFreeRingBuffer)

target_link_libraries(my_target PRIVATE LockFreeRingBuffer::LockFreeRingBuffer)
```

### Using `add_subdirectory`

If vendored or added as a Git submodule:

```cmake
add_subdirectory(path/to/LockFreeRingBuffer EXCLUDE_FROM_ALL)
target_link_libraries(my_target PRIVATE LockFreeRingBuffer::LockFreeRingBuffer)
```

> **Note:** Ensure your target enables C++20 (e.g. `set(CMAKE_CXX_STANDARD 20)` or `target_compile_features(my_target PRIVATE cxx_std_20)`).

## Usage

Include `lfrb.hpp` and instantiate `LockFreeRingBuffer<T>` with your desired capacity:

```cpp
#include <lfrb.hpp>
#include <iostream>
#include <string>

int main() {
    // Capacity rounds up to the next power of 2 (e.g., 10 -> 16)
    LockFreeRingBuffer<std::string> buffer(10);

    // push() accepts rvalues; returns false if full
    buffer.push("hello");

    std::string val = "world";
    buffer.push(std::move(val));

    // pop() takes an output reference; returns false if empty
    std::string item;
    while (buffer.pop(item)) {
        std::cout << item << '\n';
    }

    return 0;
}
```

### API Summary

| Method | Description |
| --- | --- |
| `explicit LockFreeRingBuffer(int size)` | Constructs a buffer with capacity rounded up to the nearest power of 2. |
| `bool push(T &&elem)` | Enqueues an rvalue. Returns `true` on success, `false` if the buffer is full. |
| `bool pop(T &dest)` | Dequeues the oldest element into `dest`. Returns `true` on success, `false` if empty. |

## Build & Test

To build and run the test suite locally:

```bash
# Configure
cmake -DCMAKE_BUILD_TYPE=Release -B build -S .

# Build
cmake --build build

# Run Tests
./build/tests/bench

# or
./build/tests/lfrb_tests
```

## License

This project is licensed under the [Apache License 2.0](LICENSE).
