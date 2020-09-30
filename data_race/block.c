//
// Created by bytedance on 2020/9/9.
//

#include "block.h"



pthread_mutex_t mutex; //互斥量
pthread_cond_t condp,condc; //生产者等待的信号量,消费者等待的信号量
int buffer = 0; //缓冲区

#define PRODUCT_NUM 1000

//生产者
static void* product(void* ptr) {
    for (int i=0;i<PRODUCT_NUM;i++) {
        //临界区开始
        pthread_mutex_lock(&mutex);

        //说明生产的数据还没有被消费者取走
        while (buffer != 0) {
            //使用while的原因是线程因等待条件变量陷入阻塞的状态可能被信号打断 -- EINTR
            pthread_cond_wait(&condp,&mutex);
            //当条件不满足陷入阻塞后,互斥锁会被解开,此时其他线程可以进入临界区
        }

        buffer = i;
        //唤醒消费者进行消费,但是需要注意的是,不同于信号量,为条件变量发送的通知可能会丢失
        //因此需要应用放自己处理通知丢失的场景,这个目前尚未解决
        pthread_cond_signal(&condc);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0); //线程正常退出
}

//消费者
static void* consume(void* ptr) {
    int temp;
    for (int i=0;i<PRODUCT_NUM;i++) {
        pthread_mutex_lock(&mutex);
        //说明生产者还没有生产新数据
        while (buffer == 0) {
            //使用while的原因是线程因等待条件变量陷入阻塞的状态可能被信号打断 -- EINTR
            pthread_cond_wait(&condc,&mutex);
            //当条件不满足陷入阻塞后,互斥锁会被解开,此时其他线程可以进入临界区
        }
        temp = buffer;
        buffer = 0;
        //唤醒生产者进行生产,但是需要注意的是,不同于信号量,为条件变量发送的通知可能会丢失
        //因此需要应用放自己处理通知丢失的场景,这个目前尚未解决
        pthread_cond_signal(&condp);
        pthread_mutex_unlock(&mutex);

        //在临界区外进行消费逻辑,减少临界区的长度
        printf("current_number=%d\n",temp);
    }
    pthread_exit(0); //线程正常退出
}

void product_consume_demo() {
    pthread_t prd,con;
    pthread_mutex_init(&mutex,0); //初始化互斥量
    pthread_cond_init(&condp,0); //初始化条件变量
    pthread_cond_init(&condc,0); //初始化条件变量
    pthread_create(&prd,0,product,0); //生产者线程启动
    pthread_create(&con,0,consume,0); //消费者线程启动
    //等待消费者,生产者线程执行完成
    pthread_join(prd,0);
    pthread_join(con,0);
    //销毁互斥量,条件变量
    pthread_cond_destroy(&condp);
    pthread_cond_destroy(&condc);
    pthread_mutex_destroy(&mutex);
}
