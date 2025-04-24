#pragma once
#include "Common.hpp"

#include "ThreadCache.h"
#include "PageCache.h"
static void* ConcurrentAlloc(size_t size)
{
    if(MemoryPool::pTLSThreadCache == nullptr)
    {
        MemoryPool::pTLSThreadCache = new MemoryPool::ThreadCache();
    }
    
    if(size <= MAX_BITES)
    {
        assert(MemoryPool::pTLSThreadCache);
        return MemoryPool::pTLSThreadCache->allocate(size);
    }else{
        std::cout << "申请大块内存" << std::endl;

        size_t align_size = SizeClass::roundUp(size); // 内存对齐
        size_t kpage = align_size >> PAGE_SHIFT;
        Span* ksp = MemoryPool::PageCache::getInstance()->getNPageSpan(kpage);
        void* ptr = (void*)(ksp->_id << PAGE_SHIFT);
        return ptr;
    }

}

static void ConcurrentDeAlloc(void* ptr,size_t size)
{
    if(size <= MAX_BITES)
    {
        assert(MemoryPool::pTLSThreadCache);
        MemoryPool::pTLSThreadCache->dellocate(ptr,size);
    }else{
        Span* sp = MemoryPool::PageCache::getInstance()->MapObjectToSpan(ptr);
        std::cout << "找到了sp" << std::endl;
        std::cout << "sp size:" << sp->_n << std::endl;
        MemoryPool::PageCache::getInstance()->releaseSpanToPageCache(sp);
    }

}
