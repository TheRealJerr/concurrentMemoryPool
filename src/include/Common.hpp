#pragma once
#include <iostream>
#include <vector>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <unistd.h>
#include <unordered_map>
#include <atomic>
// 管理切分好的小对象链表

#define NEXT_OBJ(obj) (*(void **)obj) // 获取头部四个字节的next指针

static const size_t MAX_BYTES = 1024 * 256;

static const size_t NFREELISTS = 208; // thread cache中桶的个数

// page哈希桶的大小
static const size_t NPAGES = 129;

static const size_t PAGE_SHIFT = 13; // 代表一页是2 ^ 13

static const size_t PAGE_SIZE = 1 << PAGE_SHIFT; // 代表一页是8 kb

static const size_t DEFAULT_PAGES = 128; // 默认page cache向堆中一次申请128页的大小

// linux环境
#ifdef __linux__

#define MALLOC(SIZE) ::sbrk(SIZE)
#define FREE(PTR) ::brk(PTR)

#if defined(__x86_64__)
using PAGE_ID = unsigned long long;
#elif defined(__i386__)
using PAGE_ID = size_t

#endif

#endif

// windows环境
#ifdef _WIN32

#include <windows.h>

#define MALLOC(SIZE) VirtualAlloc(NULL, SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#define FREE(PTR) VirtualFree(PTR, 0, MEM_RELEASE)

#if defined(_WIN64)
using PAGE_ID = unsigned long long;
#elif defined(_WIN32)
using PAGE_ID = size_t;
#endif

#endif


static const size_t MAX_BITES = 25 * 1024;

static const size_t PAGES = 129;
// 管理空间表
class FreeList
{
public:
    void push(void *obj)
    {
        assert(obj);
        NEXT_OBJ(obj) = _free_list;
        _free_list = obj;
        _size++;
    }
    void *pop()
    {
        assert(_free_list);
        void *ret = _free_list;
        _free_list = NEXT_OBJ(_free_list);
        _size--;
        return ret;
    }

    void pushRange(void *begin, void *end,size_t n)
    {
        NEXT_OBJ(end) = _free_list;
        _free_list = begin;
        _size += n;
    }

    void popRange(void*& begin,void*& end,size_t n)
    {
        assert(n <= _size);
        begin = _free_list;
        end = _free_list;
        for(size_t i = 0;i < n - 1;i++)
            end = NEXT_OBJ(end);
        _free_list = NEXT_OBJ(end);
        NEXT_OBJ(end) = nullptr;
        _size -= n;
    }
    bool empty() const { return _free_list == nullptr; }
    
    size_t maxSize() const { return _max_size; }
    
    void setMaxSize(size_t max_size) { _max_size = max_size; }

    size_t size() const { return _size; }
private:
    void *_free_list = nullptr;
    size_t _max_size = 1;
    size_t _size = 0;
};

// 对齐映射规则
class SizeClass
{


public:
    static inline size_t roundUpHelper(size_t bytes, size_t alignNum)
    {
        return ((bytes + alignNum - 1) & ~(alignNum - 1));
    }

    static inline size_t roundUp(size_t size)
    {
        if (size <= 128)
        {
            return roundUpHelper(size, 8);
        }
        else if (size <= 1024)
        {
            return roundUpHelper(size, 16);
        }
        else if (size <= 8 * 1024)
        {
            return roundUpHelper(size, 128);
        }
        else if (size <= 64 * 1024)
        {
            return roundUpHelper(size, 1024);
        }
        else if (size <= 256 * 1024)
        {
            return roundUpHelper(size, 8 * 1024);
        }
        else // 以page对齐
        {
            return roundUpHelper(size, 1 << PAGE_SHIFT);
        }
    }
    static inline size_t indexHelper(size_t bytes, size_t align_shift)
    {
        return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
    }
    // 计算映射的哪⼀个⾃由链表桶
    static inline size_t index(size_t bytes)
    {
        assert(bytes <= MAX_BYTES);
        // 每个区间有多少个桶
        static int group_array[4] = {16, 56, 56, 56};
        if (bytes <= 128)
        {
            return indexHelper(bytes, 3);
        }
        else if (bytes <= 1024)
        {
            return indexHelper(bytes - 128, 4) + group_array[0];
        }
        else if (bytes <= 81024)
        {
            return indexHelper(bytes - 1024, 7) + group_array[1] + group_array[0];
        }
        else if (bytes <= 64 * 1024)
        {
            return indexHelper(bytes - 8 * 1024, 10) + group_array[2] +
                   group_array[1] + group_array[0];
        }
        else if (bytes <= 256 * 1024)
        {
            return indexHelper(bytes - 64 * 1024, 13) + group_array[3] +
                   group_array[2] + group_array[1] + group_array[0];
        }
        else
        {
            assert(false);
        }
        return -1;
    }
    static size_t numMoveSize(size_t size)
    {
        // 慢开始的启动的算法

        // 这个算法保证了分配不会太大
        // 同时也不会太小

        assert(size > 0);
        size_t betch_size = 0;
        betch_size = MAX_BITES / size;
        if (betch_size <= 2)
            betch_size = 2;
        if (betch_size > 512)
            betch_size = 512;
        return betch_size;
    }
    // 通过大小获取页的大小
    static inline size_t numMovePage(size_t size)
    {
        size_t num = numMoveSize(size);
        size_t npage = num * size;

        npage >>= PAGE_SHIFT;

        if (npage == 0)
            npage = 1;
        return npage;
    }
};

// 管理多个大块内存块跨度结构

struct Span
{
    PAGE_ID _id;
    size_t _n; // 页的数量

    Span *_next = nullptr;
    Span *_prev = nullptr;

    size_t _use_count; // 内存切好的小块内存分配给thread cache的计数

    void *_free_list; // 切好的小块内存

    std::atomic<bool> _used;
    
    // 从这个span中获取betch_size大的内存块
    size_t fetchRangeObj(void *&begin, void *&end, size_t betch_size)
    {
        void *cur = _free_list;
        void *cur_prev = nullptr;
        size_t count = 0;
        while (count < betch_size && cur != nullptr)
        {
            cur_prev = cur;
            cur = NEXT_OBJ(cur);
            count++;
        }
        // 获取到了[_free_list,cur_prev]大的内存空间
        NEXT_OBJ(cur_prev) = nullptr;
        begin = _free_list, end = cur_prev;
        _free_list = cur;
        _use_count += count;
        return count;
    }
    void pushFront(void* ptr)
    {
        NEXT_OBJ(ptr) = _free_list;;
        _free_list = ptr;
        _use_count--; // 回收了一个小块内存
    }

    size_t useCount() const { return _use_count; }
};
