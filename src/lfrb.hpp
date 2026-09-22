#include <atomic>
#include <bit>
#include <cstdint>
#include <deque>

#pragma once

template<typename T>
struct Slot
{
    T data;
    std::atomic<int64_t> state;

    Slot(T _data, int64_t _state)
        : data(_data)
        , state(_state)
    {}
};

template<typename T>
class LockFreeRingBuffer
{
public:
    explicit LockFreeRingBuffer(int size)
        : tail(0)
        , head(0)
        , m_size(roundUpToPowerOf2(size))
        , m_mask(m_size - 1)
    {
        for (auto i = 0; i < m_size; i++) {
            slots.emplace_back(T(), i);
        }
    }

    LockFreeRingBuffer() = delete;
    LockFreeRingBuffer(const LockFreeRingBuffer &other) = delete;
    LockFreeRingBuffer(LockFreeRingBuffer &&other) = delete;
    LockFreeRingBuffer &operator=(const LockFreeRingBuffer &other) = delete;
    LockFreeRingBuffer &operator=(LockFreeRingBuffer &other) = delete;

    ~LockFreeRingBuffer() {}

    /*
     * @brief push - adds an element to the tail position.
     * @param elem - element to add. rvalue.
     * @return - success or fail (bool) for push of element.
     */
    bool push(T &&elem)
    {
        int64_t pos;
        Slot<T> *s;
        for (;;) {
            pos = tail.load(std::memory_order_relaxed);
            s = &slots[pos & m_mask];
            int64_t seq = s->state.load(std::memory_order_acquire);
            int64_t diff = seq - pos;

            if (diff == 0) {
                // Slot is ready for this position — try to claim it
                if (tail.compare_exchange_weak(pos, pos + 1, std::memory_order_acq_rel))
                    break; // We exclusively own this slot now
            } else if (diff < 0) {
                return false; // Buffer full
            }
            // diff > 0: another producer claimed it but hasn't published yet; spin
        }

        // Plain (non-atomic) write — only this thread touches this slot
        s->data = elem;

        // Publish: release ensures the data write is visible before the seq update
        s->state.store(pos + 1, std::memory_order_release);

        return true;
    }

    /*
     * @brief push - adds an element to the tail position.
     * @param elem - element to add. lvalue ref.
     * @return - success or fail (bool) for push of element.
     */
    bool push(T &elem) { return this->push(std::move(elem)); }

    /**
     * @brief pop - Removes oldest element from the head.
     * @param dest - will hold the popped element if successful.
     * @return - success or fail (bool) of pop operation.
     */
    bool pop(T &dest)
    {
        Slot<T> *s;
        int64_t pos;
        for (;;) {
            pos = head.load(std::memory_order_relaxed);
            s = &slots[pos & m_mask];
            int64_t seq = s->state.load(std::memory_order_acquire);
            int64_t diff = seq - (pos + 1);

            if (diff == 0) {
                // Data is published — try to claim the read
                if (head.compare_exchange_weak(pos, pos + 1, std::memory_order_acq_rel)) {
                    dest = s->data; // Plain read — safe, we own it
                    s->state.store(pos + m_size, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false; // Buffer empty
            }
        }
    }

private:
    const int64_t m_size;
    const int64_t m_mask;
    std::atomic<int64_t> head;
    std::atomic<int64_t> tail;
    std::deque<Slot<T>> slots;

    /**
     * @brief roundUpToPowerOf2 - Takes a number and rounds it up to the nearest power of 2 integer.
     * @param n - the number to round up.
     * @return - the nearest power of 2 integer greater than or equal to n.
     */
    uint64_t roundUpToPowerOf2(uint64_t n)
    {
        if (n == 0)
            return 1;
        if (n == 1)
            return 1;
        return 1ULL << (std::bit_width(n - 1));
    }
};
