#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <ctime>
#include "./include/ConcurrentAlloc.h"


void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds) {
    std::vector<std::thread> vthread;
    vthread.reserve(nworks);
    std::atomic<size_t> malloc_costtime = 0;
    std::atomic<size_t> free_costtime = 0;

    for (size_t k = 0; k < nworks; ++k) {
        vthread.emplace_back([&, k]() {
            std::vector<void*> v;
            v.reserve(ntimes);
            
            for (size_t j = 0; j < rounds; ++j) {
                // malloc测试
                size_t begin1 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    v.push_back(malloc(16));
                }
                size_t end1 = clock();
                
                // free测试
                size_t begin2 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    free(v[i]);
                }
                size_t end2 = clock();
                
                v.clear();
                malloc_costtime += (end1 - begin1);
                free_costtime += (end2 - begin2);
            }
        });
    }

    for (auto& t : vthread) {
        t.join();
    }

    // 转换为毫秒并输出
    double malloc_ms = static_cast<double>(malloc_costtime) / CLOCKS_PER_SEC * 1000;
    double free_ms = static_cast<double>(free_costtime) / CLOCKS_PER_SEC * 1000;
    double total_ms = malloc_ms + free_ms;

    printf("%zu个线程并发执行%zu轮次，每轮次malloc %zu次: 花费：%.2f ms\n",
           nworks, rounds, ntimes, malloc_ms);
    printf("%zu个线程并发执行%zu轮次，每轮次free %zu次: 花费：%.2f ms\n",
           nworks, rounds, ntimes, free_ms);
    printf("%zu个线程并发malloc&free %zu次，总计花费：%.2f ms\n",
           nworks, nworks * rounds * ntimes, total_ms);
}

void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds) {
    std::vector<std::thread> vthread;
    vthread.reserve(nworks);
    std::atomic<size_t> malloc_costtime = 0;
    std::atomic<size_t> free_costtime = 0;

    for (size_t k = 0; k < nworks; ++k) {
        vthread.emplace_back([&]() {
            std::vector<void*> v;
            v.reserve(ntimes);
            
            for (size_t j = 0; j < rounds; ++j) {
                // alloc测试
                size_t begin1 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    v.push_back(ConcurrentAlloc(16));
                }
                size_t end1 = clock();
                
                // dealloc测试
                size_t begin2 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    ConcurrentDeAlloc(v[i], 16);
                }
                size_t end2 = clock();
                
                v.clear();
                malloc_costtime += (end1 - begin1);
                free_costtime += (end2 - begin2);
            }
        });
    }

    for (auto& t : vthread) {
        t.join();
    }

    // 转换为毫秒并输出
    double malloc_ms = static_cast<double>(malloc_costtime) / CLOCKS_PER_SEC * 1000;
    double free_ms = static_cast<double>(free_costtime) / CLOCKS_PER_SEC * 1000;
    double total_ms = malloc_ms + free_ms;

    printf("%zu个线程并发执行%zu轮次，每轮次concurrent alloc %zu次: 花费：%.2f ms\n",
           nworks, rounds, ntimes, malloc_ms);
    printf("%zu个线程并发执行%zu轮次，每轮次concurrent dealloc %zu次: 花费：%.2f ms\n",
           nworks, rounds, ntimes, free_ms);
    printf("%zu个线程并发concurrent alloc&dealloc %zu次，总计花费：%.2f ms\n",
           nworks, nworks * rounds * ntimes, total_ms);
}

int main() {
    size_t n = 10000;
    std::cout << "==========================================================" << std::endl;
    BenchmarkConcurrentMalloc(n, 4, 10);
    std::cout << std::endl << std::endl;
    BenchmarkMalloc(n, 4, 10);
    return 0;
}