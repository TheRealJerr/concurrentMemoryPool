#pragma once
#include "Common.hpp"
#include <thread>
static const size_t MAX_BITES = 25 * 1024;

namespace MemoryPool
{
    // 每个thread cache是私有的
    class ThreadCache
    {
    public:
        ThreadCache()
        {}

        void* allocate(size_t size);
        
        void dellocate(void* ptr,size_t size);
        
        void* fetchFromCentralCache(size_t size);
    private:
        FreeList _free_lists[NFREELISTS]; // hash桶式结构来进行判断
    };
    static thread_local ThreadCache* pTLSThreadCache = nullptr; // 没有线程独立的tls 

}
// thread local storage, 针对线程的全局变量, tls