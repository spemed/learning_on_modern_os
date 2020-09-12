//
// Created by bytedance on 2020/9/12.
//


#include <zconf.h>
#include "share_memory.h"
#define PROJECT_ID 2020

_Noreturn void read_from_share_memory() {

    key_t key;
    key = ftok("./",PROJECT_ID); //根据文件路径+project_id用于构建不重复的key_t类型的返回值,提供进程唯一的标识符找到自己的共享内存段
    if (-1 == key) {
        perror("failed to create ipc key");
    }

    //打开或者创建一个共享内存段
    //IPC_CREAT--如果不存在该共享内存区段则创建,IPC_EXCL--如果存在该共享内存区段则报错
    //0666 -- 共享内存的访问权限(可读可写)
    //返回共享内存段的id或者-1(表示获取失败) [可以理解为open函数,打开文件成功后返回文件句柄]
    int shmid = shmget(key,sizeof(counter),IPC_CREAT|0666);
    if(-1 == shmid) {
        perror("failed to create share memory fragment");
    }

    //把操作系统内核中的共享内存段映射到进程内存中(让进程中的某个指针指向共享内存段)
    //第一个参数需要传递 共享内存段的id(类似文件句柄)
    //第二个参数指定共享内存在用户进程中的映射地址,为null则由操作系统自行分配(类似socket编程中客户端connect时不指定端口则由操作系统自行分配)
    //第三参数为flag位
    //  0：共享内存具有可读可写权限。
    //  SHM_RDONLY：只读。
    //  SHM_RND：（shmaddr 非空时才有效）
    void *shm_read = shmat(shmid,NULL,SHM_RDONLY); //返回一个void*指针(已经完成映射，该指针指向共享内存段)

    counter read_counter;
    memcpy(&read_counter,shm_read,sizeof(counter));
    printf("current counter=%d\n",read_counter);
}

_Noreturn void write_from_share_memory() {

    key_t key;
    key = ftok("./",PROJECT_ID); //根据文件路径+project_id用于构建不重复的key_t类型的返回值,提供进程唯一的标识符找到自己的共享内存段
    if (-1 == key) {
        perror("failed to create ipc key");
    }

    //打开或者创建一个共享内存段
    //IPC_CREAT--如果不存在该共享内存区段则创建,IPC_EXCL--如果存在该共享内存区段则报错
    //0666 -- 共享内存的访问权限(可读可写)
    //返回共享内存段的id或者-1(表示获取失败) [可以理解为open函数,打开文件成功后返回文件句柄]
    int shmid = shmget(key,sizeof(counter),IPC_CREAT|0666);
    if(-1 == shmid) {
        perror("failed to create share memory fragment");
    }

    //把操作系统内核中的共享内存段映射到进程内存中(让进程中的某个指针指向共享内存段)
    //第一个参数需要传递 共享内存段的id(类似文件句柄)
    //第二个参数指定共享内存在用户进程中的映射地址,为null则由操作系统自行分配(类似socket编程中客户端connect时不指定端口则由操作系统自行分配)
    //第三参数为flag位
    //  0：共享内存具有可读可写权限。
    //  SHM_RDONLY：只读。
    //  SHM_RND：（shmaddr 非空时才有效）
    void *shm_write = shmat(shmid,NULL,0); //返回一个void*指针(已经完成映射，该指针指向共享内存段)
    counter write_counter = 100;
    memcpy(shm_write,&write_counter,sizeof(counter));
    printf("current counter=%d\n",write_counter);
}

/**
 * 共享内存的优点
 *   1.是最快的ipc手段
 *   2.调用简单,就类似打开一个文件一样,最终返回了一个用户进程中的指针(进过映射后已经指向了操作系统的共享内存段),
 *     可以使用memcpy类函数以该指针为参数进行操作(所有操作都会在操作系统维护的共享内存段中生效)。
 * 缺点:
 *   1.访问共享区域时需要加锁进行保护,控制临界区(多个进程写时需要使用信号量等手段进行控制)
 *   2.无法投递结构化的数据,本质上是对经过映射后的操作系统的共享内存段进行操作(用户进程中存在一个指针指向该段(保存了该段的地址？))
 *     代码可读性和维护上可能并不是很清晰
 * 建议使用管道或者消息队列
 */
void run_share_memory_demo() {
    int pid = fork();
    if(-1 == pid) {
        perror("failed to create fork");
    }
    //子进程
    if (0 == pid) {
        read_from_share_memory(); //子进程启动,读取共享内存的
    //父进程
    } else {
        write_from_share_memory(); //父进程负责往共享内存中写入数据
    }
}