

#include "./include/PageCache.h"
#include "./include/CentralCache.h"

namespace MemoryPool
{
    PageCache* PageCache::_self = nullptr;

    Span* PageCache::getNPageSpan(size_t pages)
    {

        assert(pages >= 0 && pages <= NPAGES);
        // 首先找找pages位置
        {
            std::unique_lock<std::mutex> lock(_page_mtx);
            if(_page_lists[pages].empty() == false)
            {
                return _page_lists[pages].popFront();
            }
    
            // 开始切分
            for(size_t i = pages + 1;i < NPAGES;i++)
            {
                if(_page_lists[i].empty() == false)
                {
                    Span* nsp = _page_lists[i].popFront();
                    Span* ksp = new Span();
                    // 
                    // 从n个页的span中切出k个span
                    ksp->_id = nsp->_id;
                    ksp->_n = pages;
                    //
                    nsp->_id += pages;
                    nsp->_n -= pages;
                    _page_lists[nsp->_n].pushFront(nsp); // 将n - pages大小的span重新挂接上去
                    return ksp;
                }
            }
        }
        // 最开始的时候，可能什么都没有
        // pages树中可能什么都没有
        // 这个时候我们需要向堆申请一个128页的span
        Span* large_span = new Span;
        void* large_memory = ::sbrk(PAGE_SIZE * DEFAULT_PAGES);
        if(large_memory == nullptr)
            throw std::bad_alloc();
        large_span->_n = DEFAULT_PAGES;
        large_span->_id = (PAGE_ID)(large_memory) >> PAGE_SHIFT;
        {
             //
            std::unique_lock<std::mutex> lock(_page_mtx); 
            _page_lists[large_span->_n].pushFront(large_span);
        }
        return getNPageSpan(pages);// 解决这个资源缺失的问题，复用上方的代码
    }
}