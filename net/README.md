## 网络io
&emsp;&emsp;使用epoll LT触发模式，主从reactor设计。包括epoll，event loop，tcp connection，acceptor，tcp server。
### epoll
> * 对linux中epoll的封装
> * 实现对所监听fd集合及事件、回调函数的增删改
> * 实现对所监听fd注册事件的监视及回调触发
### event loop
> * 包含一个epoll，在loop循环中对sock fd集合进行监听
> * 支持同线程和跨线程添加任务
> * 通过event fd实现异步添加任务到loop循环中执行
### tcp connection
> * 一个tcp connection代表一个与客户端通信的连接
> * 一个tcp connection属于一个event loop，包含所属event loop的指针
> * 一个tcp connection属于一个tcp server，包含所属tcp server的指针
> * 一个tcp connection包含data_buf，作为应用层缓冲区收发数据
> * 可以给tcp connection设置事件和回调函数，这些将被注册到所属eventloop的epoll中被监听和触发
> * tcp connection包含定时器id，当有新的消息到来，tcp server可以通过id更新定时器中该tcp connection的时间，实现剔除超时连接
> * tcp connection中包含std::any的对象，用于对应用层协议对象状态的保存和获取，以实现对各种应用层协议的支持
### acceptor
> *  实现bind，listen，accept功能
>  * 属于一个单独的event loop，在其中执行accept任务
### tcp server
> * 使用acceptor进行bind，listen，accept
> * 拥有线程池，每个线程池中执行一个event loop，用于对tcp conn事件的监听处理
> * 拥有定时器，对tcp conn进行超时剔除
> * 使用round robin的方式，选取event loop为新来的tcp连接服务

### 测试
> * echo客户端
> * 在tcp server的基础上，实现的echo server
