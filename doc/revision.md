# YourSQL

本组程序基于 D 组代码继续开发。

## 结构设计

数据库系统的所有资源均置于 `OOPD` 名称空间中。

### `Controller`

接收并执行 SQL 语句。

### `Session`

也可以接收并执行 SQL 语句。代码由 `Controller` 改造而来，将在后面详细说明。

### `Table`

数据表。定义表的结构并存储数据，采用 B+ 树。

### `DataBase`

数据库。包含数据表的集合。

### `BPTree`, `BPTreeNode`

B+ 树及 B+ 树的结点。设计成模板类以方便类型复用。

## 为什么几乎没有修改 `Controller` 而是另外写了 `Session` 类

1. 前后端分离。`Controller` 是外壳组件，在大作业中视为「测试代码」；
2. `Controller` 对 SQL 语句的处理很差，到处都是空格和大小写判断；
3. D 组代码中将 `Controller` 和 `DataBase` 打包放到了 `OOPDB` 类中，破坏了前后端分离原则；
4. 避免过度修改 `Controller` 类造成与第一阶段 D 组的 main.cpp 和 `OOPDB` 类不兼容。

## 第二阶段新增内容

### 基础需求 (25%)

* [x] (5%) 兼容第一阶段代码（兼容 main.cpp、`OOPDB` 类和`Controller` 类）
* [x] (5%) `SELECT INTO` 保存数据库输出到文件；`LOAD DATA` 从文件加载数据表
* [ ] (5%) `COUNT()` 函数
* [ ] (5%) `GROUP BY` 分组子句
* [ ] (5%) `ORDER BY` 排序子句

### 拓展需求 (35%)

* [ ] (10%) 更多数字函数
* [x] (15%) 远程访问支持
* [ ] (10%) 存档/读档功能，且每次操作均可保存文件

## 任务分工

### 第一阶段（simple-db）

* 数据类型系统 `data_t`
* 数据库结构设计
* 外壳代码

### 第二阶段（YourSQL）

* 文件读写（基础需求）
* 网络访问 + 并发处理
