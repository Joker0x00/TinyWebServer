# HTTP层处理

## 1 HTTP解析

HTTP请求的格式如下图所示：

![](https://secure2.wostatic.cn/static/agc1EyLQSLNWrT8aXJmnaP/image.png?auth_key=1721658415-oGPf4kSqgUWACMW3Z5pBt1-0-483edb77ee138aae03186dd1c203d4cc)

HTTP请求报文遵循着规定的格式，我们只需要按要求即可准确解析。

报文分为三个部分：

- 请求行：包含请求方法（GET，POST，...），请求url和HTTP版本。中间由空格分隔，最后有个`\r\n`。
- 请求头：包含若干个请求体，由`key: value`组成，末尾有`\r\n`。最后一个请求头最后有两个`\r\n`。
- 请求体（可有可无）

```Http
GET /index.html HTTP/1.1
Host: www.example.com
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Connection: keep-alive

```

由于HTTP报文每一行最后都有一个`\r\n`，我们可以从缓冲区中搜索该字符串，将每一行截取出来再进行解析。整个过程由于分三个阶段进行，因此可使用状态机解决。为此，我们规定4个状态，分别是解析请求行、解析请求头、解析请求体和结束。在不同的状态执行不同的操作，来解析不同的内容。

## 2 HTTP响应

以下是HTTP响应报文的一个例子，主要包含响应行、响应头和响应体。

```HTML
HTTP/1.1 200 OK
Date: Fri, 19 Jul 2024 10:00:00 GMT
Server: Apache/2.4.41 (Ubuntu)
Last-Modified: Mon, 28 Jun 2024 14:30:00 GMT
Content-Type: text/html; charset=UTF-8
Content-Length: 305
Connection: close

<!DOCTYPE html>
<html>
<head>
    <title>Example Page</title>
</head>
<body>
    <h1>Welcome to Example Page</h1>
    <p>This is a sample HTML page.</p>
</body>
</html>

```

根据对HTTP请求的处理结果，生成相应的HTTP响应结果。

响应行中包含HTTP版本、响应状态码和摘要。

响应头中包含连接状态、返回的文件类型和长度。

响应体中包含返回的资源文件。


## 3 HTTP处理

HTTP处理模块是整个服务器的核心模块，负责管理客户端的连接、读写数据、HTTP处理逻辑。

- 管理连接：

  当有客户端连接时，初始化相关数据，存储fd和客户端地址。

  当由于某种原因，需要断开连接时，该模块关闭fd，重置相关数据
- 读写数据：
    - 根据不同的模式读取数据，调用buffer中的`readFd`函数。
    - 将缓冲区的数据写入socket中。此时需要注意一次可能不能将全部数据写出，需要循环写出，并更新指针。
- 负责整个HTTP的处理流程：

  首先调用ParseHttpRequest解析读缓冲区的数据，然后再调用HttpResponse生成响应报文，并放入写缓冲区中，最后将写缓冲和请求文件地址赋值给iovec结点，等待写出。