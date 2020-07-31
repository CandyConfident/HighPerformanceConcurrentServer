## Env
- Linux version 5.3.0-28-generic
- gcc version 7.5.0 
- Ubuntu 18.04.1
- cmake version 3.17.0

## Build
&emsp; ./build.sh
## Description
&emsp;&emsp;基于C++11、部分C++14/17特性的一个高性能并发网络服务器，包括目前已实现日志、线程池、内存池、定时器、网络io等模块。模块间低耦合高内聚，可作为整体也可单独提供服务。对各模块提供了单元测试。  
&emsp;&emsp;网络io使用epoll LT触发模式，采用主从reactor设计。提供同步和异步日志，内存池使用哈希表、链表结合的管理，线程池支持任意任务参数和任务结果返回，定时器使用最小堆管理、支持多执行线程、支持在指定时间后执行任务、支持周期性执行任务、支持指定时间间隔重复执行指定次数任务、支持取消定时器等。  
&emsp;&emsp;*具体设计参考各目录下readme。*
## TODO List
> * http解析设计与实现
> * 数据库连接池及缓存设计与实现
> * C++20模块与协程引入
> * 日志优化：参考[Nanolog](https://github.com/PlatformLab/NanoLog)设计，引入日志压缩、无锁队列、寄存器记录时间、减少缓存miss等方案
> * 使用lua脚本语言，配置服务器ip、port，并检查参数合法性
> * 引入gtest测试框架，整合单元测试
> * test
