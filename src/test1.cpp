#include "./include/ConcurrentAlloc.h"

void Alloc1()
{
    for(size_t i = 0;i < 5;i++)
    {
        std::cout << "开始申请内存" << std::endl;
        void* ptr = ConcurrentAlloc(6);
        (void)ptr;
        if(ptr) std::cout << "应用层申请成功" << std::endl;
        
        // ConcurrentDeAlloc(ptr,6);
    }
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
void TLSTest()
{
    std::thread t1(Alloc1);
    std::thread t2(Alloc2);
    if(t1.joinable()) t1.join();
    if(t2.joinable()) t2.join();
} 

int main()
{
    TLSTest();
    return 0;
}