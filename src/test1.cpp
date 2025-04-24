#include "./include/ConcurrentAlloc.h"
#include <iostream>
#include <vector>

class Data
{
public:
    Data(int data) : _data(data)
    {}
    int getData() const { return _data; }
private:
    int _data;
};

void Alloc1()
{
    std::vector<void*> v;
    void* ptr1 = ConcurrentAlloc(6);
    void* ptr2 = ConcurrentAlloc(6);
    void* ptr3 = ConcurrentAlloc(6);
    void* ptr4 = ConcurrentAlloc(6);
    void* ptr5 = ConcurrentAlloc(6);
    void* ptr6 = ConcurrentAlloc(6);
    void* ptr7 = ConcurrentAlloc(6);

    std::cout << (void*) ptr1 << std::endl;
    std::cout << (void*) ptr2 << std::endl;
    std::cout << (void*) ptr3 << std::endl;
    std::cout << (void*) ptr4 << std::endl;
    std::cout << (void*) ptr5 << std::endl;
    std::cout << (void*) ptr6 << std::endl;
    std::cout << (void*) ptr7 << std::endl;

    ConcurrentDeAlloc(ptr1,6);
    ConcurrentDeAlloc(ptr2,6);
    ConcurrentDeAlloc(ptr3,6);
    ConcurrentDeAlloc(ptr4,6);
    ConcurrentDeAlloc(ptr5,6);
    ConcurrentDeAlloc(ptr6,6);
    ConcurrentDeAlloc(ptr7,6);
}
void Alloc2()
{
    for(size_t i = 0;i < 7;i++)
    {

        void* ptr = ConcurrentAlloc(6);
        (void)ptr;
        // ConcurrentDeAlloc(ptr,6);
    }
}

void testTheadAlloc()
{
    std::vector<void*> v;
    for(size_t i = 0;i < 7;i++)
        v.push_back(ConcurrentAlloc(6));
    for(auto* ptr : v)
        ConcurrentDeAlloc(ptr,6);

}
void testBigAlloc()
{
    void* ptr1 = ConcurrentAlloc(257 * 1024);

    void* ptr2 = ConcurrentAlloc(129 * PAGE_SIZE);

    ConcurrentDeAlloc(ptr1,257 * 1024);
    ConcurrentDeAlloc(ptr2,129 * PAGE_SIZE);
}
void TLSTest()
{
    std::thread t1(testTheadAlloc);
    std::thread t2(testTheadAlloc);
    if(t1.joinable()) t1.join();
    if(t2.joinable()) t2.join();

} 


int main()
{
    std::thread t(testBigAlloc);
    if(t.joinable()) t.join();
    return 0;
}