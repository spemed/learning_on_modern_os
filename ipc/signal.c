//
// Created by bytedance on 2020/11/2.
//
#include <zconf.h>
#include "signal.h"
#include <unistd.h>
#include "stdio.h"
/**
    ipc之信号
    --信号通常用于进程之间的异步通信，实际上是用软件模拟了硬件中断。信号可以由一个进程发给另一个进程或者由内核发给某个进程，当进程捕获到某个进程时
      会按照程序设置的值或者默认值进行回调。[这可能打断某个系统调用,通常会返回EINTER错误]
        --1.每个信号都可以关联一个回调函数用于在信号达到时被调用
        --2.信号除了SIGKILL和SIGSTOP外都可以被忽略,被忽略的信号到达时进程不做任何响应
        --3.信号可以设置默认行为[设置SIG_DEF],默认通常是在收到信号后终止进程,某些信号还会在进程的工作目录产生内存映像
        --4.信号是易失的，当进程正在处理某个信号的回调时[会对当前信号进行阻塞操作[如果到达了新的同种信号,也不会发给当前进程]]。
            多个相同信号陆续到达，在信号解阻塞后，也只会收到一个信号。
        --5.SA_RESTART[标志位]。如果设置了该标志位，那么当进程从信号中断的回调退出后,操作系统内核会自动恢复被信号打断的系统调用(慢系统调用)
    --根据实践观察,信号函数的回调是单线程的(如果你的进程中就只有单线程的话)。向进程发出两个信号,只有在第一个回调函数退出后,才会执行第二个回调函数。
      所以不可以在信号函数中出现占用cpu时间过长的代码或者死循环，否则会线程的其他代码。[研究多线程监听信号可解决这个问题]
      而信号的阻塞和解阻塞在单线程下难以验证，因为无论解不解阻塞，信号函数都会等待上一个信号函数执行完毕在执行。
      信号的易失性可以验证,在信号回调函数sleep 5s的情况下，在5s内连续发送5个相同信号,发现只有两个信号得到了响应。
      第一个响应是信号函数第一次接收到信号后的执行，而后连续发送的四个信号中，信号函数却只执行了一次。这是因为在信号函数sleep的时候。
      有且只有一个信号会被保留，多余的信号会被抛弃,信号是易失的，仅能保证至少收到一次，这一点需要注意。
*/
static sigfunc *signal_setter(int signo,sigfunc* func) {
    struct sigaction act,oact;
    act.sa_handler = func;
    sigemptyset(&(act.sa_mask)); //把sa_mask设置为空集,意味在信号函数处理的过程中,不阻塞额外的信号

    // 在信号函数处理的过程中,阻塞SIGABRT信号。当信号函数处理完,解阻塞SIGABRT信号。
    // 但是该信号是可能丢失的,多次递交SIGABRT也只有一个信号会发送给进程
    // sigaddset(&(act.sa_mask),SIGABRT);
    act.sa_flags = 0; //初始化标识位
    //如果当前信号为时钟信号[时钟中断,可以实现定时器]
    //跨平台兼容
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT; //sunos 4.x
#endif
    } else {
#ifdef SA_RESTART
        //意味当信号回调执行完会恢复被打断的慢系统调用
        act.sa_flags |= SA_RESTART; //srv4 4.4BSD
#endif
    }
    if(sigaction(signo,&act,&oact) < 0) {
        return (SIG_ERR);
    }
    return oact.sa_handler;
}

sigfunc *test(int signo) {
    sleep(10);
    printf("wait 10s,current signo=%d\n",signo);
    return NULL;
}

sigfunc *test2(int signo) {
    sleep(5);
    printf("wait 5s,current signo=%d\n",signo);
    return NULL;
}

void signal_show() {
    printf("%d\n",getpid());
    signal_setter(SIGSYS,test);
    signal_setter(SIGABRT,test2);
    while (1) {
        sleep(5000); //阻塞进程
    }
}