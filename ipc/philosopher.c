//
// Created by bytedance on 2021/2/12.
//

#include <pthread.h>
#include <stdio.h>
#include "philosopher.h"

//获取叉子
void get_fork();
//放下叉子
void put_fork();
//测试能否取得叉子,无法取得则阻塞直到其他哲学家让出叉子
static void test();



/**
 * 哲学家线程执行的函数体
 * @param tid 线程运行时传入的参数
 */
static void* handle(void *t_index) {
    printf("i am the %d philosopher\n",*((int *)t_index));
    return NULL;
}

_Noreturn void run() {
    //每个线程模拟一个哲学家的行为
    pthread_t people_numbers[PEOPLE];
    for (int i = 0; i < PEOPLE; ++i) {
        int b = i;
        pthread_create(&people_numbers[i],NULL,handle,&b);
    }
    //死循环阻塞主进程
    while(1);
}