#include <vector>

#pragma once

template <typename T>
class LockFreeRingBuffer {
public:
    explicit LockFreeRingBuffer(int size)
        : m_size(size)
    {
        elems.reserve(size);
    }

    LockFreeRingBuffer() = delete;
    LockFreeRingBuffer(const LockFreeRingBuffer &other) = delete;
    LockFreeRingBuffer(LockFreeRingBuffer &&other) = delete;
    LockFreeRingBuffer &operator=(const LockFreeRingBuffer &other) = delete;
    LockFreeRingBuffer &operator=(LockFreeRingBuffer &other) = delete;

    ~LockFreeRingBuffer() {}

    /**
     * @brief push - adds an element to the tail position.
     * @param elem - element to add. Only rvalues accepted.
     */
    void push(T &&elem) {}

    /**
     * @brief pop - Removes oldest element from the head.
     * @param dest - will hold the popped element if successful.
     * @return - success or fail (bool) of pop operation.
     */
    bool pop(T &dest) {}

private:
    int m_size;
    std::vector<T> elems;
};
