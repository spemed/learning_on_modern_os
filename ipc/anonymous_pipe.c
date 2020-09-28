//
// Created by bytedance on 2020/9/13.
//

#include "anonymous_pipe.h"

#define READ_PIPE 0
#define WRITE_PIPE 1
static int pipe_fd[2]; //分别创建读管道和写管道

void read_from_anonymous_pipe(int read_pipe,char *r_buf,int r_buf_len) {
    int r_num;
    while (1) {
        r_num=read(read_pipe,r_buf,r_buf_len);
        if (r_num == -1) {
            if (EINTR != errno) {
                perror("failed to read buf from read_pipe");
                exit(-1);
            }
        } else {
            printf("read %d bytes from father progress,data is %d\n",r_num,atoi(r_buf));
            break;
        }
    }
}

void write_to_anonymous_pipe(int write_pipe,char *w_buf,int w_buf_len) {
    int r_num;
    while (1) {
        //往管道中写入w_buf,如果管道已满则陷入阻塞,这也是个慢系统调用。需要考虑被软件中断(信号)打断后从write返回
        //返回-1说明写入失败,如果errno != EINTR。则报错后退出
        r_num = write(write_pipe,w_buf,w_buf_len);
        if (r_num == -1) {
            if (EINTR != errno) {
                printf("w_buf=%s\n",w_buf);
                perror("failed to send buf to write_pipe");
                exit(-1);
            }
        } else {
            printf("write %d bytes to son progress,data is %s\n",r_num,w_buf);
            break;
        }
    }
}

/**
 * 匿名管道
 *
 * 管道实际是用于进程间通信的一段共享内存(由内核维护)，
 * 创建管道的进程称为管道服务器，连接到一个管道的进程为管道客户端。
 * 一个进程在向管道写入数据后，另一进程就可以从管道的另一端将其读取出来。

　　管道的特点：

　　（1）管道是半双工的，数据只能向一个方向流动；需要双方通信时，需要建立起两个管道；

　　（2）只能用于父子进程或者兄弟进程之间（具有亲缘关系的进程）。
        比如fork或exec创建的新进程，在使用exec创建新进程时，需要将管道的文件描述符作为参数传递给exec创建的新进程。
        当父进程与使用fork创建的子进程直接通信时，发送数据的进程关闭读端，接受数据的进程关闭写端。

　　（3）单独构成一种独立的文件系统：管道对于管道两端的进程而言，就是一个文件，但它不是普通的文件，它不属于某种文件系统，而是自立门户，单独构成一种文件系统，并且只存在与内存中。

　　（4）数据的读出和写入：一个进程向管道中写的内容被管道另一端的进程读出。写入的内容每次都添加在管道缓冲区的末尾，并且每次都是从缓冲区的头部读出数据。

　　管道的实现机制：

　　管道是由内核管理(所以fork之后可以跨进程共享)的一个缓冲区，管道的一端连接一个进程的输出。这个进程会向管道中写入信息。管道的另一端连接一个进程的输入，这个进程取出被放入管道的信息。
   一个缓冲区不需要很大，它被设计成为环形(循环队列)的数据结构，以便管道可以被循环利用。
   当管道中没有信息的话，从管道中读取的进程会等待，直到另一端的进程放入信息。
   当管道被放满信息的时候，尝试放入信息的进程会等待，直到另一端的进程取出信息。
   当两个进程都终结的时候，管道也自动消失。

　　管道只能在本地计算机中使用，而不可用于网络间的通信。(只能用于本地主机进程(父子,兄弟)之间的ipc)
 */

/**
 * demo实现的是父进程写,子进程读的场景
 */
void run_anonymous_pipe_demo() {

    pid_t pid;
    char r_buf[10];
    bzero(r_buf,sizeof(r_buf));
    char w_buf[4];
    bzero(w_buf,sizeof(w_buf));

    if(0 != pipe(pipe_fd))
    {
        perror("failed to create pipe");
        exit(-1);
    }

    if((pid = fork()) < 0)  {
        perror("failed to fork");
        exit(-1);
    }

    //子进程
    if (0 == pid) {
        close(pipe_fd[WRITE_PIPE]); //子进程关闭写管道,引用计数减一
        read_from_anonymous_pipe(pipe_fd[READ_PIPE],r_buf,sizeof(r_buf));
        close(pipe_fd[READ_PIPE]); //子进程关闭写管道,引用计数减一
        exit(1);
    } else {
        close(pipe_fd[READ_PIPE]); //父进程关闭读管道,引用计数减一
        strcpy(w_buf,"123"); //把123复制到w_buf数组中(逐字节复制到数组中)
        write_to_anonymous_pipe(pipe_fd[WRITE_PIPE],w_buf,(int )strlen(w_buf));
        close(pipe_fd[WRITE_PIPE]);
        exit(1);
    }
}