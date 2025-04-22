#include "./include/ThreadCache.h"
#include "./include/CentralCache.h"
namespace MemoryPool
{
    void* ThreadCache::allocate(size_t size)
    {
        size_t rounde_up_size = SizeClass::roundUp(size); // 内存对齐
        size_t index = SizeClass::index(size);
        if(_free_lists[index].empty() == false) return _free_lists[index].pop();
        // 向central cache中进行获取
        return fetchFromCentralCache(index,rounde_up_size);// 测试代码
    }

    void ThreadCache::dellocate(void* ptr,size_t size)
    {
        size_t index = SizeClass::index(size);
        _free_lists[index].push(ptr);
    }   
    // 这里拿到的size已经是内存对齐后的size
    void* ThreadCache::fetchFromCentralCache(size_t index,size_t size)
    {
        // 这里类似于tcp连接时候的慢增长模式
        std::cout << "从central cache申请数据" << std::endl;
        size_t betch_size = std::min(_free_lists[index].maxSize(),SizeClass::numMoveSize(size));
        if(_free_lists[index].maxSize() == betch_size)
            _free_lists[index].setMaxSize(betch_size + 1); // 慢增长
        // betch_size就是我们申请的内存块
        void* begin = nullptr, *end = nullptr;
        size_t actual_size = CentralCache::getInstance()->fetchRangeObj(begin,end,betch_size,size);
        assert(actual_size >= 1); // 至少给我一个内存吧
        std::cout << "申请到了：" << actual_size << "大的内存" << std::endl;
        if(actual_size == 1)
        {
            assert(begin == end && begin != nullptr);
            return begin;
        }
        else
        {
            // 将多余的数据返回我们的thread cache
            _free_lists[index].pushRange(NEXT_OBJ(begin),end);
            return begin;
        }
        throw std::bad_alloc();
        return nullptr;
    }
}