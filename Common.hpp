#pragma once
#include <iostream>
#include <vector>
#include <ctime>
#include <cassert>

// 管理切分好的小对象链表

#define NEXT_OBJ(obj) (*(void **)obj) // 获取头部四个字节的next指针

static const size_t MAX_BYTES = 1024 * 256;

static const size_t NFREELISTS = 208; // thread cache中桶的个数

// 管理空间表  
class FreeList
{
public:
    void push(void *obj)
    {
        assert(obj);
        NEXT_OBJ(obj) = _free_list;
        _free_list = obj;
    }
    void *pop()
    {
        assert(_free_list);
        void *ret = _free_list;
        _free_list = NEXT_OBJ(_free_list);
        return ret;
    }

    bool empty() const { return _free_list == nullptr; }

private:
    void *_free_list = nullptr;
};

// 对齐映射规则
class SizeClass
{

    // 计算出对齐数
    static size_t inline roundUpHelper(size_t size, size_t alignment_number)
    {
        size_t align_size = 0;
        if (size / alignment_number != 0)
            align_size = (size / alignment_number + 1) * alignment_number;
        else
            align_size = size;
        return align_size;
    }

    
public:
   
    static size_t inline roundUp(size_t size)
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
        else
        {
            assert(false);
            return 0;
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
};