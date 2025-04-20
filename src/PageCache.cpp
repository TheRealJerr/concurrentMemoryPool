#pragma once

#include "./include/PageCache.h"
#include "./include/CentralCache.h"

namespace MemoryPool
{
    PageCache* PageCache::_self = nullptr;

    Span* PageCache::getNPageSpan(size_t pages)
    {
        assert(pages >= 0 && pages <= NPAGES);
        // 首先找找pages位置
        if(_page_lists[pages].empty() == false)
        {
            return _page_lists[pages].popFront();
        }

        // 开始切分
        for(size_t i = pages + 1;i < NPAGES;i++)
        {
            if(_page_lists[i].empty() == false)
            {
                Span* sp = _page_lists[i].popFront();
                
            }
        }
    }
}