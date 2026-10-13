#include "../include/lockfree_pool.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

struct Transaction {
    std::uint64_t id;
    double amount;
    char sender[32];
    char receiver[32];

    Transaction(std::uint64_t i, double a) : id(i), amount(a) {}
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "   Lock-Free Memory Pool Allocator (C++20) Test\n";
    std::cout << "====================================================\n\n";

    constexpr std::size_t PoolSize = 100000;
    core::memory::LockFreeMemoryPool<Transaction, PoolSize> pool;

    constexpr int NumThreads = 8;
    constexpr int OpsPerThread = 10000;
    std::vector<std::thread> workers;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < NumThreads; ++t) {
        workers.emplace_back([&pool, t]() {
            for (int i = 0; i < OpsPerThread; ++i) {
                Transaction* tx = pool.construct(t * OpsPerThread + i, 99.99);
                // Simulate work
                std::this_thread::yield();
                pool.destroy(tx);
            }
        });
    }

    for (auto& thread : workers) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    std::cout << "Successfully executed " << (NumThreads * OpsPerThread) << " allocations/deallocations.\n";
    std::cout << "Time elapsed: " << elapsed.count() << " ms\n";
    std::cout << "Active allocations remaining in pool: " << pool.active_allocations() << "\n";

    return 0;
}
