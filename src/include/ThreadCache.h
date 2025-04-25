#pragma once
#include "Common.hpp"
#include <thread>


namespace MemoryPool
{

    // 每个thread cache是对于线程私有的,通过thread_local(tls)进行管理
    class ThreadCache
    {
    public:
        ThreadCache()
        {}

        void* allocate(size_t size);
        
        void dellocate(void* ptr,size_t size);
        
        void* fetchFromCentralCache(size_t index,size_t size);

        void listTooLong(FreeList& freelist,size_t size);
    private:
        FreeList _free_lists[NFREELISTS]; // hash桶式结构来进行判断
    };
    class ThreadCacheDeltor
    {
    public:
        void operator()(ThreadCache* ptr)
        {
            gspace_creater.delThreadCache(ptr);
            std::cout << "执行线程析构器" << std::endl;
        }
    };
    extern thread_local std::unique_ptr<ThreadCache,ThreadCacheDeltor> pTLSThreadCache; // 没有线程独立的tls 

}
// thread local storage, 针对线程的全局变量, tls