<!--
 * @Author: 凉屋 liangwu.lxy@foxmail.com
 * @Description:
-->

# simple_basic_components

## 介绍
基于 C/C++ 实现的简单基础组件

## 组件


### 基础工具组件

|组件|状态|文件名|
|---|---|---|
|timer|完成|src/timer.cpp|
|reactor|未完成|src/reactor.c|
|trycatch|完成|src/trycathc.c|
|基于数组的无锁队列|完成|util/ArrayLockFreeQueue.cpp|


### 容器组件
|组件|状态|文件名|
|---|---|---|
|ringbuf|完成|util/ringbuf.*|
|queue|完成|util/queue.*|
|blk_list|未完成|util/blk_list.*|
|rbtree|完成|util/rbtree.*|
|btree|未完成|src/btree.*|
|timewheel|完成|src/timewheel.*|
|skiplist|未完成|util/skiplist.*|

### 池式组件
|组件|状态|文件名|
|---|---|---|
|thread_pool|完成|pool/thread_pool.c|
|thread_pool_cpp|未完成|pool/thread_pool.cpp|
|mmpool|完成|pool/mmpool.*|
|request_pool| 完成 | pool/asyn_dns_client.c |



