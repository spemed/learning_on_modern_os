//
// Created by bytedance on 2020/9/26.
//


#include <sys/errno.h>
#include <zconf.h>
#include "semaphore.h"

//信号量有两种实现：传统的System V信号量和新的POSIX信号量
//传统的System V常用于进程间通信,由操作系统提供的系列semget,sem_wait,semctl等函数完成工作。但是比较难用。由操作系统内核提供的类型共享内存的方式进行访问
//POSIX信号量既可以用于进程间通信也可以用于线程间通信。
//其分为有名信号量以及无名信号量。
//对于有名信号量,就算是不同进程只要进程知道信号量的名称就可以实现跨进程访问[猜测应该是存储于文件系统中] #include "sys/sem.h"
//对于无名信号量,一般用于进程间不同线程的通信[猜测是直接存在内存中,进程退出时释放资源]。但是无名信号量相关函数已经被标记为废弃,所以这里使用有名信号量完成demo
//#include <semaphore.h>
#define LOOP_NUMBER  100
void semaphore_demo(){
   int pid = fork();
   sem_t* mutex = sem_open("mutex", O_CREAT, 0666, 1);
   if(pid < 0) {
       perror("failed in fork new progress");
       exit(-1);
   } else if (0 == pid) { //如果是子进程
       int result,i=0;
       while (i < LOOP_NUMBER) {
           result = sem_wait(mutex);
           if (result < 0 && errno != EINTR) {
               perror("failed in wait mutex");
               exit(-1);
           }
           printf("say hello from progress B\n");
           result = sem_post(mutex);
           if (result < 0) {
               perror("failed in post mutex");
               exit(-1);
           }
           i++;
       }
   } else { //如果是父进程
       int result;
       int i = 0;
       while (i < LOOP_NUMBER) {
           result = sem_wait(mutex);
           if (result < 0 && errno != EINTR) {
               perror("failed in wait mutex");
               exit(-1);
           }
           printf("say hello from progress A\n");
           result = sem_post(mutex);
           if (result < 0) {
               perror("failed in post mutex");
               exit(-1);
           }
           i++;
       }
   }
}