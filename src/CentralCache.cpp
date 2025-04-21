#include "./include/CentralCache.h"
#include "./include/PageCache.h"
namespace MemoryPool
{
    // 针对声明进行定义
    CentralCache* CentralCache::_self = nullptr;

    void SpanList::insert(SpanList::iterator pos,Span* newpos)
    {
        assert(pos && newpos);
        // 
        std::unique_lock<std::mutex> lock(_mtx);
        Span* prev = pos->_prev;
        prev->_next = newpos; newpos->_prev = prev;
        pos->_prev = newpos; newpos->_next = pos;
    }

    void SpanList::erase(SpanList::iterator pos)
    {
        assert(pos && (pos != _head));
        std::unique_lock<std::mutex> lock(_mtx);
        pos->_prev->_next = pos->_next;
        pos->_next->_prev = pos->_prev;
    }

    size_t SpanList::fetchRangeObj(void*& begin,void*& end,size_t betch_size,size_t size)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        Span* sp = getOneSpan(size); // 获取一个有效的span*指针
        return sp->fetchRangeObj(begin,end,betch_size);
    }
    void SpanList::pushFront(Span* sp)
    {
        insert(begin(),sp);
    }

    Span* SpanList::popFront()
    {
        Span* ret = _head->_next;
        erase(begin());
        return ret;
    }

    
    Span* SpanList::getOneSpan(size_t size) 
    {
        iterator it = begin();
        while(it != end())
        {
            if(it->_free_list) return it;
            it = it->_next;
        }
        // 找不到通过page cache进行申请
        // fetchRangeFromPages
        Span* sp = PageCache::getInstance()->getNPageSpan(SizeClass::numMovePage(size));
        // 算出span的其实地址
        char* page_start_address = (char*)(sp->_id << PAGE_SHIFT);
        // 计算大块内存的大小
        size_t bytes = sp->_n << PAGE_SHIFT;
        // 将大块内存切成自由列表挂起来
        char* page_end_address = page_start_address + bytes;
        // 进行切割
        sp->_free_list = page_start_address;
        
        void* cur = page_start_address + size;
        void* prev = sp->_free_list;

        while(cur < page_end_address)
        {
            NEXT_OBJ(prev) = cur;
            prev = cur;
            cur += size;
        }
    }
    // 从freelist[index]中获取span并且从span中获取
    size_t CentralCache::fetchRangeObj(void*& begin,void*& end,size_t betch_size,size_t size)
    {
        size_t index = SizeClass::index(size);
        assert(betch_size > 0); // 至少获取一个大小的内存块
        return _span_lists[index].fetchRangeObj(begin,end,betch_size,size);
    }

} // namespace MemoryPool
