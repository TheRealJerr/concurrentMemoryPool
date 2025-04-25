#include "./include/Common.hpp"
#include "./include/ThreadCache.h"

Span* SpaceCreater::newSpan()
{
    return _sp_pool.New();
}
MemoryPool::ThreadCache* SpaceCreater::newThreadCache()
{
    return _tc_pool.New();
}

void SpaceCreater::delSpan(Span* ptr)
{
    _sp_pool.Delete(ptr);
}

void SpaceCreater::delThreadCache(MemoryPool::ThreadCache* ptr)
{
    _tc_pool.Delete(ptr);
}