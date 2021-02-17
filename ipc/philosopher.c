//
// Created by bytedance on 2021/2/12.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "philosopher.h"

void *alloc(size_t size){

    void *new_mem;
    //请求所需的内存，并检查确实分配成功
    new_mem = malloc(size);
    if(new_mem == NULL)
    {
        printf("Out of memory!/n");
        exit(1);
    }
    return new_mem;
}



#define MALLOC(type,size) (type*)alloc(sizeof(type)*size)

//互斥锁
sem_t *mutex;
//标识哲学家状态
int state[PEOPLE];
//哲学家的信号量
sem_t* N[PEOPLE];

//测试能否取得叉子,无法取得则阻塞直到其他哲学家让出叉子
static void test(int i) {
    if (state[i] == HUNGRY && state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING) {
        state[i] = EATING;
        sem_post(N[i]);
    }
}

//获取叉子
void get_fork(int i) {
    state[i] = HUNGRY;
    sem_wait(mutex);
    test(i);
    sem_post(mutex);
    //
    sem_wait(N[i]);
}

//放下叉子
void put_fork(int i) {
    state[i] = THINKING;
    //放下左手的叉子
    sem_post(N[LEFT(i)]);
    //放下右手的叉子
    sem_post(N[RIGHT(i)]);
}


 /**
 * 哲学家线程执行的函数体
 * @param tid 线程运行时传入的参数
 */
 _Noreturn static void* handle(void *t_index) {
    int number = *((int *)t_index);
    free(t_index);
    //todo 解决i重复的问题
    printf("i am the %d philosopher, i am thinking\n",number);
    while (1) {
        get_fork(number);
        printf("i am the %d philosopher, i am eating\n",number);
        put_fork(number);
    }
}


_Noreturn void run() {
    sem_unlink("philosopher_mutex");
    mutex = sem_open("philosopher_mutex",O_CREAT,0666,1);
    char str[255] = {'\0'};
    for (int i = 0; i < PEOPLE; ++i) {
        snprintf(str,30,"philosopher_fork_%d",i);
        sem_unlink(str);
        N[i] = sem_open(str,O_CREAT,0666,0);
    }
    //每个线程模拟一个哲学家的行为
    pthread_t people_numbers[PEOPLE];
    for (int i = 0; i < PEOPLE; ++i) {
        int* temp = MALLOC(int,1);
        *temp = i;
        pthread_create(&people_numbers[i],NULL,handle,temp);
    }
    //死循环阻塞主进程
    while(1);
}