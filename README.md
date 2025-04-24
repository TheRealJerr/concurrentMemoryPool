# concurrentMemoryPool

> create by hrj on 4.27 2025

效仿tcmalloc实现的高并发内存池

tcmalloc主要分为了三层

## thread cache

每个线程私有一份256kb内存,如果是小内存,可以直接通过thread cache获取,如果不够,可以通过central cache的接口获取

## central cache

central cache采取懒汉模式(第一次获取时初始化),由于其中hash桶的大小是固定的,所以设置桶锁,不同线程如果访问的是不同
的hash桶,不会出现资源竞争的问题

## page cache

page cache中维护了一个根据页数作业键值的定长hash表,通过页号的管理，可以减少外内存碎片的问题

##

如果size < 256 kb我们直接找三层缓存

如果size > 256 kb && size < 128 * 8k， 可以直接访问page cache

如果 > 128 * 8k , 直接使用sbrk开辟内存