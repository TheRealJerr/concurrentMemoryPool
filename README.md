# concurrentMemoryPool

效仿tcmalloc实现的高并发内存池

tcmalloc主要分为了三层

## thread cache

每个线程私有一份256kb内存,如果是小内存,可以直接通过thread cache获取,如果不够,可以通过central cache的接口获取

## central cache

central cache采取懒汉模式(第一次获取时初始化),由于其中hash桶的大小是固定的,所以设置桶锁,不同线程如果访问的是不同
的hash桶,不会出现资源竞争的问题

## page cache


