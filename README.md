# http-server
基于asio的高性能http服务器，支持Windows和Linux(5.22+)
-

- 接口简单易用
```C++
    #include "http/server.hpp"
    http::server server{8080};

    server.get("/hello", [](const http::request &) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok; //200
        res.content.emplace<std::string>("<h1>Hello world!</h1>");
        co_return res;
    });

    //正则表达式
    server.get("/.*\\.html", [](const http::request &req) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.headers.insert({"Content-Type", "text/html"});
        res.content.emplace<std::filesystem::path>(std::string{R"(../public)"} + req.url);
        co_return res;
    });

    server.listen();
```

- 高性能：使用 __IOCP__ 或 __io_uring__，小文件传输 __1万 QPS__，速率 __1.2__ __GB__ /s，大文件传输速率高达 __1.3__ __GB__ /s

<table>
    <tr>
        <th colspan=2 bgcolor=white>Server Software:</th>
        <td colspan=2 bgcolor=white></td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Server Hostname:</th>
        <td colspan=2 bgcolor=white>127.0.0.1</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Server Port:</th>
        <td colspan=2 bgcolor=white>8080</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Document Path:</th>
        <td colspan=2 bgcolor=white>/girl.jpeg</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Document Length:</th>
        <td colspan=2 bgcolor=white>119513 bytes</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Concurrency Level:</th>
        <td colspan=2 bgcolor=white>100</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Time taken for tests:</th>
        <td colspan=2 bgcolor=white>9.576 seconds</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Complete requests:</th>
        <td colspan=2 bgcolor=white>100000</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Failed requests:</th>
        <td colspan=2 bgcolor=white>0</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Total transferred:</th>
        <td colspan=2 bgcolor=white>11955800000 bytes</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>HTML transferred:</th>
        <td colspan=2 bgcolor=white>11951300000 bytes</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Requests per second:</th>
        <td colspan=2 bgcolor=white>10442.85</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Transfer rate:</th>
        <td colspan=2 bgcolor=white>1219264.17 kB/s received</td>
    </tr>
    <tr>
        <th bgcolor=white colspan=4>Connection Times (ms)</th>
    </tr>
    <tr>
        <th bgcolor=white>&nbsp;</th>
        <th bgcolor=white>min</th>
        <th bgcolor=white>avg</th>
        <th bgcolor=white>max</th>
    </tr>
    <tr>
        <th bgcolor=white>Connect:</th>
        <td bgcolor=white> 0</td>
        <td bgcolor=white> 0</td>
        <td bgcolor=white> 10</td>
    </tr>
    <tr>
        <th bgcolor=white>Processing:</th>
        <td bgcolor=white> 6</td>
        <td bgcolor=white> 9</td>
        <td bgcolor=white> 216</td>
    </tr>
    <tr>
        <th bgcolor=white>Total:</th>
        <td bgcolor=white> 6</td>
        <td bgcolor=white> 9</td>
        <td bgcolor=white> 226</td>
    </tr>
</table>

<table>
    <tr>
        <th colspan=2 bgcolor=white>Server Software:</th>
        <td colspan=2 bgcolor=white></td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Server Hostname:</th>
        <td colspan=2 bgcolor=white>127.0.0.1</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Server Port:</th>
        <td colspan=2 bgcolor=white>8080</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Document Path:</th>
        <td colspan=2 bgcolor=white>/girl.mp4</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Document Length:</th>
        <td colspan=2 bgcolor=white>11165483 bytes</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Concurrency Level:</th>
        <td colspan=2 bgcolor=white>10</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Time taken for tests:</th>
        <td colspan=2 bgcolor=white>80.413 seconds</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Complete requests:</th>
        <td colspan=2 bgcolor=white>10000</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Failed requests:</th>
        <td colspan=2 bgcolor=white>0</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Total transferred:</th>
        <td colspan=2 bgcolor=white>111655270000 bytes</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>HTML transferred:</th>
        <td colspan=2 bgcolor=white>111654830000 bytes</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Requests per second:</th>
        <td colspan=2 bgcolor=white>124.36</td>
    </tr>
    <tr>
        <th colspan=2 bgcolor=white>Transfer rate:</th>
        <td colspan=2 bgcolor=white>1355984.25 kB/s received</td>
    </tr>
    <tr>
        <th bgcolor=white colspan=4>Connection Times (ms)</th>
    </tr>
    <tr>
        <th bgcolor=white>&nbsp;</th>
        <th bgcolor=white>min</th>
        <th bgcolor=white>avg</th>
        <th bgcolor=white>max</th>
    </tr>
    <tr>
        <th bgcolor=white>Connect:</th>
        <td bgcolor=white> 0</td>
        <td bgcolor=white> 0</td>
        <td bgcolor=white> 2</td>
    </tr>
    <tr>
        <th bgcolor=white>Processing:</th>
        <td bgcolor=white> 56</td>
        <td bgcolor=white> 80</td>
        <td bgcolor=white> 136</td>
    </tr>
    <tr>
        <th bgcolor=white>Total:</th>
        <td bgcolor=white> 56</td>
        <td bgcolor=white> 80</td>
        <td bgcolor=white> 138</td>
    </tr>
</table>

---
Build
-
Linux 下依赖liburing，内核版本5.22以上
```shell
git clone https://github.com/axboe/liburing.git
cd liburing
./configure
make install
```
CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)
project(yourproject)

set(CMAKE_CXX_STANDARD 20) #支持协程

find_package(Boost 1.80.0 REQUIRED)        #依赖boost库，Linux下要求v1.78以上(io_uring)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(http)

add_executable(yourproject ...)
target_link_libraries(yourproject http) 
```
```shell
mkdir build
cd build
cmake .. 
make
./http-server
```
---
测试
-
[http://localhost:8080/index.html](http://localhost:8080/index.html)

benchmark
-
```shell
ab -n 1000 -c 10 http://127.0.0.1/girl.mp4
```