## WebBench
&emsp;&emsp;由[Lionbridge](http://www.lionbridge.com)公司开发的web服务器压测工具。
### 功能
&emsp;&emsp;测试相同硬件上不同服务的性能和不同硬件上同一服务的运行状况。
&emsp;&emsp;指标：每秒钟响应请求数和每秒钟传输数据量。

### 使用示例
&emsp; ./webbench --help
&emsp; ./webbench -c 5000 -t 120 http://www.163.com