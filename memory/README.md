## 内存池
&emsp;&emsp;内存池包括memory pool，chunk和data_buf。
### memory pool
> * 使用单例的模式
> * 以哈希表的结构管理不同大小的chunk组成的链表
> * 分配内存时，找到距离最近的chunk进行分配
> * 回收时把chunk挂回对应链表的头部
> * 当链表上没有chunk可用时申请新的chunk分配出去
> * 分配内存时向上取整
> * unique_lock和lock_guard最大的不同是unique_lock不需要始终拥有关联的mutex，而lock_guard始终拥有mutex。
> * std::unique_lock 与std::lock_guard都能实现自动加锁与解锁功能，但是std::unique_lock要比std::lock_guard更灵活，但是更灵活的代价是占用空间相对更大一点且相对更慢一点。
> * std::unique_lock相对std::lock_guard更灵活的地方在于在等待中的线程如果在等待期间需要解锁mutex，并在之后重新将其锁定。而std::lock_guard却不具备这样的功能。

### chunk
> * 内存池分配内存的单位
> * 内存池管理的链表中的一个节点
### data_buf
> * 应用层缓冲区的数据结构
> * 实际上是一个由内存池管理的chunk
> * 支持数据到data_buf，data_buf到socket文件的双向流动
### 内存池测试
> * 对memory pool分配回收chunk块的测试
> * 对数据经过data_buf到文件fd的双向流动测试