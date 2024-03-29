# YourSQL

## 服务器/客户端用法

启动服务器

```bash
./main -s
```

启动客户端

```bash
./main -c
```

指定 IP 和端口（以服务器为例，客户端同理）

```bash
./main -s -i 123.45.67.89 -p 2333
```

* 默认 IP：`127.0.0.1`
* 默认端口：`2333`

### 注意

~~目前数据库服务器虽然支持多个连接，且具有一定（也很弱）的并发应对能力（通过命令队列实现），但仍存在问题，具体表现如下：~~

~~「当前数据库」在目前的整个数据库系统中只有一个，因此多个客户端同时执行 `USE` 语句切换到不同的数据库时会引发后续基于表的操作混乱，引发运行错误。该问题将在之后通过分离 Controller 和 Database 的方式解决（这样每个客户端的会话将彼此独立）。~~

上述问题现已解决。

## 接下来的任务

### 基础需求 (25%)

* [x] (5%) 兼容第一阶段代码（主要是兼容原 main.cpp 以及原 Controller 类）
* [x] (2.5%) `SELECT INTO` 保存数据库输出到文件
* [x] (2.5%) `LOAD DATA` 从文件加载数据表
* [x] (5%) `COUNT()` 函数
* [x] (5%) `GROUP BY` 分组子句（貌似要配合 `COUNT` 一起做）
* [x] (5%) `ORDER BY` 排序子句（但仍存在问题——排序时需视 NULL 为最小值）

### 拓展需求 (35%)

* [x]  (10%) 更多数字函数（反正也得写 `COUNT`）**（已经全部写完并测试完毕）**
* [x] (15%) 远程访问支持（处理好并发及多用户同时 `USE` 的问题后应该就可以了）
* [x] (5%+5%) 存档/读档功能；没有必要一定降低复杂度（因为已经满 35% 了）

### 其他任务

* [x] 分离 Controller 和 Database（需要写新的类；目前的 Controller 仍然保留，用于兼容旧代码）
* [x] 让 Controller 具有一定的容错能力（`DROP` 不存在、`CREATE` 已有等）
* [x] 用本组第一阶段的命令解释器替换目前的命令解释器（写的实在太烂了，可能需要顺便改造某些函数，配合第一条，差不多就是重写了一个 Controller）
* [x] 去掉用来判断 NULL 的 magic number（谁知道会不会真的有人输入这些数据）
* [x] 修改排序函数，使 NULL 在排序时被认为是最小值
