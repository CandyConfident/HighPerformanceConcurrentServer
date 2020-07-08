## 内存池
&emsp;&emsp;内存池包括memory pool，chunk和data_buf。
### memory pool
> * 使用单例的模式
> * 以哈希表的结构管理不同大小的chunk组成的链表
> * 分配内存时，找到距离最近的chunk进行分配
> * 回收时把chunk挂回对应链表的头部
> * 当链表上没有chunk可用时申请新的chunk分配出去
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