#include "Common.hpp"

#include "ThreadCache.h"

static void* ConcurrentAlloc(size_t size)
{
    if(MemoryPool::pTLSThreadCache == nullptr)
    {
        MemoryPool::pTLSThreadCache = new MemoryPool::ThreadCache();
    }
    
    return MemoryPool::pTLSThreadCache->allocate(size);
}

static void ConcurrentDeAlloc(void* ptr,size_t size)
{
    assert(size <= MAX_BITES); 
    assert(MemoryPool::pTLSThreadCache);
    MemoryPool::pTLSThreadCache->dellocate(ptr,size);
}
