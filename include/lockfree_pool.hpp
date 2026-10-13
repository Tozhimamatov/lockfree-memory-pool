#ifndef LOCKFREE_POOL_HPP
#define LOCKFREE_POOL_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>
#include <concepts>
#include <stdexcept>

namespace core::memory {

/**
 * @brief High-performance, lock-free memory pool allocator using C++20 concepts and atomic ABA protection.
 * @tparam T Type of object to allocate.
 * @tparam BlockSize Number of elements per memory block chunk.
 */
template <typename T, std::size_t BlockSize = 1024>
class LockFreeMemoryPool {
private:
    union Node {
        T element;
        Node* next;

        Node() {}
        ~Node() {}
    };

    struct TaggedPointer {
        Node* ptr{nullptr};
        std::uintptr_t tag{0};
    };

    alignas(64) std::atomic<TaggedPointer> free_head_{TaggedPointer{nullptr, 0}};
    alignas(64) std::atomic<std::size_t> allocated_count_{0};

    Node* arena_{nullptr};

public:
    using value_type = T;

    explicit LockFreeMemoryPool() {
        arena_ = static_cast<Node*>(::operator new[](sizeof(Node) * BlockSize, std::align_val_t{alignof(Node)}));
        
        // Initialize free list chain
        for (std::size_t i = 0; i < BlockSize - 1; ++i) {
            arena_[i].next = &arena_[i + 1];
        }
        arena_[BlockSize - 1].next = nullptr;

        free_head_.store(TaggedPointer{&arena_[0], 0}, std::memory_order_relaxed);
    }

    ~LockFreeMemoryPool() noexcept {
        ::operator delete[](arena_, std::align_val_t{alignof(Node)});
    }

    LockFreeMemoryPool(const LockFreeMemoryPool&) = delete;
    LockFreeMemoryPool& operator=(const LockFreeMemoryPool&) = delete;
    LockFreeMemoryPool(LockFreeMemoryPool&&) noexcept = delete;
    LockFreeMemoryPool& operator=(LockFreeMemoryPool&&) noexcept = delete;

    /**
     * @brief Allocates raw memory for an object in O(1) lock-free time complexity.
     */
    [[nodiscard]] T* allocate() {
        TaggedPointer old_head = free_head_.load(std::memory_order_acquire);
        TaggedPointer new_head;

        do {
            if (old_head.ptr == nullptr) [[unlikely]] {
                throw std::bad_alloc();
            }
            new_head.ptr = old_head.ptr->next;
            new_head.tag = old_head.tag + 1; // Mitigate ABA Problem
        } while (!free_head_.compare_exchange_weak(
            old_head, new_head,
            std::memory_order_release,
            std::memory_order_acquire));

        allocated_count_.fetch_add(1, std::memory_order_relaxed);
        return reinterpret_cast<T*>(old_head.ptr);
    }

    /**
     * @brief Deallocates memory back to the pool in O(1) lock-free time complexity.
     */
    void deallocate(T* ptr) noexcept {
        if (!ptr) return;

        Node* node = reinterpret_cast<Node*>(ptr);
        TaggedPointer old_head = free_head_.load(std::memory_order_acquire);
        TaggedPointer new_head;

        do {
            node->next = old_head.ptr;
            new_head.ptr = node;
            new_head.tag = old_head.tag + 1;
        } while (!free_head_.compare_exchange_weak(
            old_head, new_head,
            std::memory_order_release,
            std::memory_order_acquire));

        allocated_count_.fetch_sub(1, std::memory_order_relaxed);
    }

    /**
     * @brief Constructs object in-place using perfect forwarding.
     */
    template <typename... Args>
    [[nodiscard]] T* construct(Args&&... args) {
        T* memory = allocate();
        ::new (static_cast<void*>(memory)) T(std::forward<Args>(args)...);
        return memory;
    }

    /**
     * @brief Destroys object and returns memory to the pool.
     */
    void destroy(T* ptr) noexcept {
        if (ptr) {
            ptr->~T();
            deallocate(ptr);
        }
    }

    [[nodiscard]] std::size_t active_allocations() const noexcept {
        return allocated_count_.load(std::memory_order_relaxed);
    }
};

} // namespace core::memory

#endif // LOCKFREE_POOL_HPP
