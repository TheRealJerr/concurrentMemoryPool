#include "ThreadCache.h"

namespace MemoryPool
{
    void* ThreadCache::allocate(size_t size)
    {
        size_t rounde_up_size = SizeClass::roundUp(size); // 内存对齐
        size_t index = SizeClass::index(size);
        if(_free_lists[index].empty() == false) return _free_lists[index].pop();
        // 向central cache中进行获取
        return fetchFromCentralCache(size);// 测试代码
    }

    void ThreadCache::dellocate(void* ptr,size_t size)
    {
        size_t index = SizeClass::index(size);
        _free_lists[index].push(ptr);
    }   
    void* ThreadCache::fetchFromCentralCache(size_t size)
    {
        // 这里简单默认是malloc
        std::cout << "malloc开辟成功" << std::endl;
        return malloc(size);
        
    }
}