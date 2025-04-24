

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
                Span* ret = _page_lists[pages].popFront();
                ret->_used = true;
                return ret;
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
                    ksp->_used = true;
                    ksp->_id = nsp->_id;
                    ksp->_n = pages;
                    //
                    nsp->_id += pages;
                    nsp->_n -= pages;
                    _page_lists[nsp->_n].pushFront(nsp); // 将n - pages大小的span重新挂接上去
                    // 添加映射关系
                    for(PAGE_ID i = ksp->_id;i < (ksp->_id + ksp->_n);i++)
                        _id_span_map[i] = ksp;
                    // nsp还在page cache中,因此为了方便合并，我们只需要映射前后页
                    _id_span_map[nsp->_id] = nsp;
                    _id_span_map[nsp->_id + nsp->_n - 1] = nsp; 
                    //
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
        // 这里不用再次添加hash映射,因为会复用上方的切分代码
        return getNPageSpan(pages);// 解决这个资源缺失的问题，复用上方的代码
    }

    Span* PageCache::MapObjectToSpan(void* ptr)
    {
        PAGE_ID id = (PAGE_ID)ptr >> PAGE_SHIFT;
        if(_id_span_map.count(id) == 0) assert(false);
        return _id_span_map[id];
    }   
    // 将span归还给page cache合并
    // 我们要将sp中的头部和尾部进行合并
    void PageCache::releaseSpanToPageCache(Span* sp)
    {
        std::unique_lock<std::mutex> lock(_page_mtx);
        // 对sp的前后页进行合并缓解内存碎片的问题   
        // 这里不能直接使用use_count
        // 1. 当前页可以处于正在切分的状态
        // 2. 可能刚好还回来,还没来得即还到page cache
        // 合并的大小不能找过128 页
        // 1. 往前合并
        while(true)
        {
            // 一直往前扩
            PAGE_ID prev_id = sp->_id - 1;
            auto it = _id_span_map.find(prev_id);
            if(it->second->_used == true || it == _id_span_map.end()) break;
            // 这个时候说明是可以合并的
            if(sp->_n + it->second->_n > 128) break; // 超过128就不合并了
            
            sp->_id = it->second->_id;
            sp->_n += it->second->_n;
            _page_lists[it->second->_n].erase(it->second);
            // 删除hash映射
            // 切分的时候会重构映射
            _id_span_map.erase(it->second->_id);
            delete it->second;
        }

        // 1. 向后合并
        while(true)
        {
            // 一直往后扩
            PAGE_ID next_id = sp->_id + sp->_n;
            auto it = _id_span_map.find(next_id);
            if(it->second->_used == true || it == _id_span_map.end()) break;
            // 这个时候说明是可以合并的
            if(sp->_n + it->second->_n > 128) break; // 超过128就不合并了
            
            sp->_n += it->second->_n;
            _page_lists[it->second->_n].erase(it->second);
            // 删除hash映射
            // 切分的时候会重构映射
            _id_span_map.erase(it->second->_id);
            delete it->second;
        }
        // 这个时候sp是一个很大的sp
        _page_lists[sp->_n].pushFront(sp);
        // 方便进行合并
        sp->_used = false;
        _id_span_map[sp->_id] = sp;
        _id_span_map[sp->_id + sp->_n - 1] = sp;
    }

}