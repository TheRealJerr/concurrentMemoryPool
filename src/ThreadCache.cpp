#include "./include/ThreadCache.h"
#include "./include/CentralCache.h"
namespace MemoryPool
{
    thread_local std::unique_ptr<ThreadCache,ThreadCacheDeltor> pTLSThreadCache = nullptr;
    void* ThreadCache::allocate(size_t size)
    {
        size_t rounde_up_size = SizeClass::roundUp(size); // 内存对齐
        size_t index = SizeClass::index(size);
        if(_free_lists[index].empty() == false) return _free_lists[index].pop();
        // 向central cache中进行获取
        return fetchFromCentralCache(index,rounde_up_size);// 测试代码
    }

   
    // 这里拿到的size已经是内存对齐后的size
    void* ThreadCache::fetchFromCentralCache(size_t index,size_t size)
    {
        // 这里类似于tcp连接时候的慢增长模式
        size_t betch_size = std::min(_free_lists[index].maxSize(),SizeClass::numMoveSize(size));
        if(_free_lists[index].maxSize() == betch_size)
            _free_lists[index].setMaxSize(betch_size + 1); // 慢增长
        // betch_size就是我们申请的内存块
        void* begin = nullptr, *end = nullptr;
        
        size_t actual_size = 0;
        while(true)
        {
            actual_size = CentralCache::getInstance()->fetchRangeObj(begin,end,betch_size,size);
            if(actual_size >= 1) break; // 重复申请
        }
        if(actual_size == 1)
        {
            assert(begin == end);
            assert(begin != nullptr);
            assert(end != nullptr);
            return begin;
        }
        else
        {
            // 将多余的数据返回我们的thread cache
            size_t count = 1;
            void* test = begin;
            while(test != end) 
            {
                count++;
                test = NEXT_OBJ(test);
            }
            assert(count == actual_size);
            _free_lists[index].pushRange(NEXT_OBJ(begin),end,actual_size - 1);
            return begin;
        }
        throw std::bad_alloc();
        return nullptr;
    }

    void ThreadCache::dellocate(void* ptr,size_t size)
    {
        size_t index = SizeClass::index(size);
        _free_lists[index].push(ptr);

        // 当链表的长度大于一次批量申请内存的时候,释放内存给central cache
        if(_free_lists[index].size() >= _free_lists[index].maxSize())
        {
            listTooLong(_free_lists[index],size);
        }
    }   
    // 从free_list中拿出一个批量的内存块
    void ThreadCache::listTooLong(FreeList& freelist,size_t size)
    {
        void* start = nullptr, *end = nullptr;
        freelist.popRange(start,end,freelist.maxSize());
        CentralCache::getInstance()->releaseListToSpan(start,size);
    }

}