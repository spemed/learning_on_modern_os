//
// Created by bytedance on 2020/9/5.
// 创建10个线程打印hello world
//

#include "hello_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static void* print_hello_world(void *tid) {
    printf("Hello world.Greetings from thread %d\n",*((int*)tid)); //马上刷新行缓冲区,重定向到标准输出
    pthread_exit(NULL); //进程退出
}

/**
 *
 * @param thread_num
 * 会依次创建thread_num个线程
 * 但是打印出来并不会按照顺序,也不会从 0到thread_num 打印出Greetings from thread,打印时当前i的值是多少,就会输出多少
 * 造成这个原因是因为多线程可能会在多核上并行,导致程序执行的步骤并不确定,所以需要同步机制(信号量,条件变量)
 * @result
 *  create thread,tid=0
    create thread,tid=1
    create thread,tid=2
    create thread,tid=3
    create thread,tid=4
    Hello world.Greetings from thread 4
    Hello world.Greetings from thread 4
    create thread,tid=5
    Hello world.Greetings from thread 5
    create thread,tid=6
    Hello world.Greetings from thread 4
    Hello world.Greetings from thread 6
    Hello world.Greetings from thread 4
    create thread,tid=7
    Hello world.Greetings from thread 7
    Hello world.Greetings from thread 8
    create thread,tid=8
    create thread,tid=9
 */
void run_demo(int thread_num) {
    int status;
    pthread_t* pthread_set = malloc(sizeof(pthread_t)*thread_num);
    for(int i=0;i<thread_num;i++) {
        printf("create thread,tid=%d\n",i);
        //status返回0说明创建成功
        status = pthread_create(&pthread_set[i],NULL,print_hello_world,(void*)&i);
        if (status != 0) {
            printf("failed to create pthread tid=%d,status code=%d",i,status);
            break;
        }
    }
    free(pthread_set); //释放堆内存
}

/**
 * peterson算法
 */