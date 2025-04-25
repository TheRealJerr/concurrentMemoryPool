#pragma once
#include <iostream>
using std::cout;
using std::endl;
#include <unistd.h>
// 定长内存池的实现
#include "Common.hpp"
namespace MemoryPool
{
    const size_t default_object_size = 1024 * 128;
    template <class T>
    class ObjectPool
    {
    public:
        ObjectPool(size_t size = default_object_size) : // 默认申请128 kb 的大小
                                                _free_list(nullptr)
        {
            _memory = (char*)MALLOC(size);
            if (_memory == nullptr)
                throw std::bad_alloc();
            _remain_bytes = size;
        }
        template <class ...Args>
        T *New(Args&&... args)
        {

            T *obj = nullptr;
            if (_free_list)
            {
                void *next = *(void **)_free_list;
                obj = (T*)_free_list;
                _free_list = next;
            }
            else
            {
                if (_remain_bytes <= sizeof(T))
                {
                    _memory = (char*)MALLOC(default_object_size);
                    if (_memory == nullptr)
                        throw std::bad_alloc();
                    _remain_bytes = default_object_size;
                }
                obj = (T *)_memory;
                size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
                _memory += objSize;
                _remain_bytes -= objSize;
            }
            new(obj) T(std::forward<Args>(args)...);
            return obj;
        }

        void Delete(T *obj)
        {
            // |sizeof(void*)|      |   将释放内存的前sizeof(void*)的字段设置成为指向下一个位置next指针
            obj->~T();// 显示调用obj的析构函数
            *(void **)obj = _free_list;
            _free_list = (void *)obj;
        }
        size_t getFreeListSize() const 
        {
            // 获取freeList的长度
            size_t count = 0;
            void* tmp = _free_list;
            while(tmp)
            {
                tmp = *(void**)_free_list;
                count++;
            }
            return count;
        }
    private:
        char *_memory;        // 可用内存的起始点
        size_t _remain_bytes; // 剩余的内存大小
        void *_free_list;     // 管理释放的内存
    };
}