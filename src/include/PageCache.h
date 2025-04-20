#include "Common.hpp"
#include "./include/CentralCache.h"
namespace MemoryPool
{
    
    class PageCache
    {
        PageCache() = default;
        PageCache(const PageCache&) = delete;
        PageCache& operator==(const PageCache&) = delete;
    public:
        // 懒汉模式
        static PageCache* getInstance() 
        {
            if(_self == nullptr)
                _self = new PageCache();
            return _self;
        }

        // 获取一个pages页的span
        Span* getNPageSpan(size_t pages);

        // void releasePage(SpanList* begin,SpanList* end)
    private:
        // 这里hash桶的下标代表的是需要的页数
        // _page_lists[n]表示n页的span链表
        SpanList _page_lists[NPAGES];
        std::mutex _mtx;  //
        
        static PageCache* _self;
    };
    // 这里不用的桶锁的原因是,我们申请并不局限于
    // 一个桶,我们可以向下继续寻找
    // 同时也是方便页的合并
}