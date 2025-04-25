#pragma once
#include "Common.hpp"
// 中心缓存
#include <mutex>
namespace MemoryPool
{

    // 由于spanList被复用与page cache和 central cache,
    // 所以需要针对不同的场景
    enum class Choice
    {
        NO_LOCK = 0,
        LOCK,
    };
    // 双向循环链表
    class SpanList
    {
    public:
        using iterator = Span*;
        SpanList(): _head(new Span())
        {
            _head->_next = _head;
            _head->_prev = _head;
        }
        void insert(iterator pos,Span* newSpan);

        void erase(iterator pos);

        void pushFront(Span* sp);

        Span* popFront();
        
        Span* getOneSpan(size_t size);

        size_t fetchRangeObj(void*& begin,void*& end,size_t betch_size,size_t size);
        
        
        // 提供迭代器进行遍历
        iterator begin() { return _head->_next; }

        iterator end() { return _head; }

        bool empty() const { return _head->_next == _head; }

        void releaseToSpan(void* start);
    private:
        iterator _head;
        std::mutex _mtx; // 定义桶锁,不同的桶互相安全
    };
    // 如果我们的ThreadCache中没有可用的内存,就通过fetchFromCentralCache向central cache申请内存
    // central通过span为单位进行申请
    // span是以多个page为单位的大块内存
    // 由于这里的hash桶的大小是定长的,因此可以使用桶锁，针对每个桶单独加锁,这样可以显著加快不同内存块的申请

    class CentralCache
    {
        CentralCache()
        {}

        CentralCache(const CentralCache& ) = delete;

        CentralCache& operator==(const CentralCache&) = delete;
    public:
        // 懒汉模式
        static CentralCache* getInstance()
        {
            return &_self;
        }
        // 返回值是实际给的
        size_t fetchRangeObj(void*& obj,void*& end,size_t betch_size,size_t size);
        // 获取一个非空的span

        void releaseListToSpan(void* start,size_t bytes);
    private:
        // 定义桶锁
        SpanList _span_lists[NFREELISTS];
        static CentralCache _self;
    };  
}