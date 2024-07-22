# 1 主体框架

客户端如果想和服务器通信，首先要和服务器建立TCP连接，然后发送HTTP请求。服务端接收并处理HTTP请求，然后发送HTTP响应。

![](https://secure2.wostatic.cn/static/aWqaeqvKMXpBHDZcCNcF9c/image.png?auth_key=1721658093-hFFTxX7yAkoVUumkveSTGF-0-cdc6765afb016a97036f928ef9e405a1)

服务器采用单Reactor多线程模式，主线程使用IO多路复用接口监听事件，收到事件后将其分发。连接事件直接处理，而读写事件由线程池负责处理。

> Reactor模式介绍：[https://xiaolincoding.com/os/8_network_system/reactor.html#单-reactor-多线程-多进程](https://xiaolincoding.com/os/8_network_system/reactor.html#单-reactor-多线程-多进程)

![](https://secure2.wostatic.cn/static/36NMCs6YxLtbofCdET9SG3/image.png?auth_key=1721658093-aShF43VVCbFjYa8eXUvCHh-0-44a188374e36e0116e1e08d0310f2348)

项目主要包含以下模块：

- Server层-基于EPOLL的I/O多路复用和Reactor网络模式
- HTTP处理层-解析HTTP请求并处理，生成返回的HTTP响应
- 日志系统
- 线程池
- 数据库连接池
- 定时器
- 缓冲区-暂存缓冲数据，读写socket

```Bash

TinyWebServer
├── build
│   ├── bin
│   │   └── server
│   └── Makefile
├── CMakeLists.txt
├── config.ini
├── log
├── Makefile
├── resources
├── webbench-1.5
└── webserver
    ├── buffer
    │   ├── Buffer.cpp
    │   └── Buffer.h
    ├── http
    │   ├── HttpResponse.cpp
    │   ├── HttpResponse.h
    │   ├── HttpWork.cpp
    │   ├── HttpWork.h
    │   ├── ParseHttpRequest.cpp
    │   └── ParseHttpRequest.h
    ├── lib
    │   └── inih-r58
    ├── log
    │   ├── Log.cpp
    │   ├── Log.h
    │   ├── LogLevel.h
    │   └── LogQueue.h
    ├── main.cpp
    ├── pool
    │   ├── sqlconnpool.cpp
    │   ├── sqlconnpool.h
    │   ├── sqlconnRAII.h
    │   ├── ThreadPool.cpp
    │   └── ThreadPool.h
    ├── server
    │   ├── Epoll.cpp
    │   ├── Epoll.h
    │   ├── Server.cpp
    │   └── Server.h
    ├── timer
    │   ├── Timer.cpp
    │   └── Timer.h
    └── utils
        └── Utils.h
```

# 2 压力测试
## 2.1 ET模式

```Bash
./webbench-1.5/webbench -c 5000 -t 10 http://127.0.0.1:20001/
```

![](https://secure2.wostatic.cn/static/v33NdqDqe2najyviucj64/image.png?auth_key=1721658263-2GDM8VEteMN5gjvSuDmzXK-0-8020ae5e955ea39f505e1c9d3aa843ef)

```Bash
./webbench-1.5/webbench -c 8000 -t 10 http://127.0.0.1:20001/
```

![](https://secure2.wostatic.cn/static/cEgKD3kURz5dd9p6ihG6NV/image.png?auth_key=1721658263-24Fe8n3F94CPtzApq9saNA-0-da0e6ca0e9286bdb86f7f40c0143717f)

```Bash
./webbench-1.5/webbench -c 10000 -t 10 http://127.0.0.1:20001/

```

![](https://secure2.wostatic.cn/static/4dWQdxY33BYZ492rxJBKyZ/image.png?auth_key=1721658263-BTv2Qm5yeGRdqHdpAE2Mc-0-ca5812c4312d8a7e0d82114a5b9240c4)

## 2.2 LT模式

```Bash
./webbench-1.5/webbench -c 10000 -t 10 http://127.0.0.1:20001/
```

![](https://secure2.wostatic.cn/static/fF4TA5tRZCNSXDMkFQmxvX/image.png?auth_key=1721658263-k8XcGSz1tooQvni6KnwtSq-0-35508fa11a63dfc38c3419bc1ade7bcf)
## 2.3 测试环境
* Ubuntu: 20.04
* cpu: i5-1035G1
* 内存: 16G

# 3 运行说明

进入项目根目录

```Bash
make
./build/bin/server
```

# 4 参考资料
> https://github.com/markparticle/WebServer \
> Linux高性能服务器编程，游双著.