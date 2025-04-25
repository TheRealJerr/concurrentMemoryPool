#include "./include/CentralCache.h"
#include "./include/PageCache.h"
namespace MemoryPool
{
    // 针对声明进行定义
    CentralCache CentralCache::_self;

    void SpanList::insert(SpanList::iterator pos, Span *newpos)
    {
        assert(pos && newpos);
        //
        Span *prev = pos->_prev;
        prev->_next = newpos;
        newpos->_prev = prev;
        pos->_prev = newpos;
        newpos->_next = pos;
    }

    void SpanList::erase(SpanList::iterator pos)
    {
        assert(pos != nullptr);
        assert(pos != _head);
        std::unique_lock<std::mutex> lock(_mtx);
        pos->_prev->_next = pos->_next;
        pos->_next->_prev = pos->_prev;
    }

    size_t SpanList::fetchRangeObj(void *&begin, void *&end, size_t betch_size, size_t size)
    {
        // 能够保证一定获取到一个有效的Span指针
        // 将桶锁锁上
        _mtx.lock();
        Span *sp = getOneSpan(size); // 获取一个有效的span*指针
        // 现在一个重要的问题就是sp可能我们获得的时候已经为空了
        // if(sp && sp->_free_list) ELOG("获取到了一个合理的span,请求多大的内存:%d",(int)(betch_size));
        auto ret = sp->fetchRangeObj(begin, end, betch_size);
        sp->_use_count += ret;
        // 检查一下
        void* test = begin;
        size_t t = 1;
        while(test != end)
        {
            test = NEXT_OBJ(test);
            t++;
        }
        assert(t == ret);
        // 释放铜锁
        _mtx.unlock();
        return ret;
    }
    void SpanList::pushFront(Span *sp)
    {
        insert(begin(), sp);
    }

    Span *SpanList::popFront()
    {
        Span *ret = _head->_next;
        erase(begin());
        return ret;
    }
    void SpanList::releaseToSpan(void *start)
    {
        _mtx.lock();
        
        while (start)
        {
            void *next = NEXT_OBJ(start);
            auto* sp = PageCache::getInstance()->MapObjectToSpan(start);
            sp->pushFront(start);
            // 说明某个Span所有的小的内存块都回收成功
            if(sp->useCount() == 0)
            {
                // std::cout << "释放给page cache" << std::endl;
                _mtx.unlock();// 这里是一个坑, erase是线程安全的,所以erase之前应该释放锁
                erase(sp);// 
                _mtx.lock();
                sp->_free_list = nullptr;
                sp->_prev = sp->_next = nullptr;

                // 释放span的时候,使我们的page cache进行操作,这段时间内,我们允许其他线程访问central cache进行资源的获取和释放
                _mtx.unlock(); // 将锁进行解除
                PageCache::getInstance()->releaseSpanToPageCache(sp);
                _mtx.lock();
            }
            start = next;
        }
        _mtx.unlock();
    }

    Span *SpanList::getOneSpan(size_t size)
    {
        // 申请资源加锁
        // 从现在的桶中寻找是否有合适的Span模块
        iterator it = begin();
        while (it != end())
        {
            if (it->_free_list)
                return it;
            it = it->_next;
        }
        // 这个时候我们合理应该释放锁，方便其他线程归还资源
        // 找不到通过page cache进行申请
        // fetchRangeFromPages
        // ELOG("central cache中数据不足,通过page cache申请页数:%d",(int)SizeClass::numMovePage(size));
        _mtx.unlock();
        Span *sp = PageCache::getInstance()->getNPageSpan(SizeClass::numMovePage(size));
        // if(sp && sp->_free_list) ELOG("从page cache中申请成功,span的页数:%d,对齐内存的大小:%d",(int)sp->_n,(int)size);
        // 算出span的其实地址
        char *page_start_address = (char *)(sp->_id << PAGE_SHIFT);
        // 计算大块内存的大小
        size_t bytes = sp->_n << PAGE_SHIFT;
        // 将大块内存切成自由列表挂起来
        char *page_end_address = page_start_address + bytes;
        // 进行切割
        sp->_free_list = (void *)page_start_address;
        char *cur = page_start_address + size;
        void *prev = sp->_free_list;
        while (cur < page_end_address)
        {
            NEXT_OBJ(prev) = cur;
            prev = cur;
            cur += size;
        }
        NEXT_OBJ(prev) = nullptr;
        // 这个时候添加获取到的span
        _mtx.lock();
        // 这里不用加锁应为push Front是线程安全的(里面加了锁,如果加锁就会死锁)
        pushFront(sp);

        return sp;
    }
    // 从freelist[index]中获取span并且从span中获取
    size_t CentralCache::fetchRangeObj(void *&begin, void *&end, size_t betch_size, size_t size)
    {
        size_t index = SizeClass::index(size);
        assert(betch_size > 0); // 至少获取一个大小的内存块
        return _span_lists[index].fetchRangeObj(begin, end, betch_size, size);
    }
    void CentralCache::releaseListToSpan(void *start, size_t bytes)
    {
        size_t index = SizeClass::index(bytes);
        _span_lists[index].releaseToSpan(start);
    }
} // namespace MemoryPool
