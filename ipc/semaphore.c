//
// Created by bytedance on 2020/9/26.
//

#include <stdio.h>
#include "semaphore.h"
#include <stdlib.h>

/**
基于信号量的ipc手段一般是用于提供进程间的同步机制
*/
void semaphore_demo(){
    //创建信号量
     int sem = semget(3,1,IPC_CREAT | IPC_EXCL);
     if (sem < 0) {
         perror("failed to create new sem");
         exit(-1);
     }
    sem_wait(&sem);
    sem_wait(&sem);
    sem_post(&sem);
    printf("完成!");
}