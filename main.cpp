#include "ObjectPool.h"

using namespace MemoryPool;
#include <vector>
class Data
{
public:
    Data(int data): _data(data)
    {}

    int getData() const { return _data; }
private:
    int _data;
};
int main()
{
    ObjectPool<std::vector<int>> obj_pool;
    auto* t = obj_pool.New(10,0);
    obj_pool.Delete(t);
    std::cout << obj_pool.getFreeListSize() << std::endl;
    t = obj_pool.New(19,1);
    for(auto e : *t)
        std::cout << e << " ";
    std::cout << std::endl;
    obj_pool.Delete(t);
    std::cout << obj_pool.getFreeListSize() << std::endl;
    return 0;
}