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
    for(size_t i = 0;i < 7;i++)
    {
        v.push_back(ConcurrentAlloc(6));
    }
    for(auto* ptr : v)
        ConcurrentDeAlloc(ptr,6);
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

    if(t1.joinable()) t1.join();

} 

int main()
{
    TLSTest();
    return 0;
}