//
// Created by bytedance on 2020/9/9.
// cpu忙等待的处理模式在一定程度上降低了cpu的利用率。
// 我们希望当进程获取不到进入临界区的资格时，而且需要等待相当长的一段时间时，
// (如果等待的时间很短,cpu轮询反而好过让进程调度,因为陷入到内核以及恢复进程执行的cpu现场信息都是有时间开销的)，
// 可以把进程挂起，调度其他进程到cpu上执行。
// 这样可以大大提升cpu的利用率
// 基于这个思路,又有几种解法
//   1.基于信号 (基于异步的ipc,但是信号可能会丢失)
//   2.基于信号量 （基于同步目的的ipc,信号量存储在操作系统的共享区域/共享文件中(有system V和posix V),跨进程也可以访问）
//   3.互斥量和条件变量(主要用于线程的同步)
//   基于信号量的在ipc的semaphore有涉及,这里展示使用pthread实现线程同步的样例

#ifndef OS_BLOCK_H
#define OS_BLOCK_H

#include <zconf.h>
#include <pthread.h>
#include <stdio.h>

static void* product(void*);
static void* consume(void*);
void product_consume_demo(void);
#endif //OS_BLOCK_H
