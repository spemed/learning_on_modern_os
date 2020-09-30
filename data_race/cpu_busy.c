//
// Created by bytedance on 2020/9/6.
//

#include <pthread.h>
#include "cpu_busy.h"
#include <unistd.h>

/**
 * 什么是竞争条件？
 *  因为进程执行顺序的不确定性，在访问共享资源(共享内存,共享文件等等)发生了丢失修改，重复计算等行为使得程序最终执行的结果和预期不符的行为
 * 什么是临界区和临界资源？
 *  临界区是指在同一时刻只允许一个进程进入的代码段,其他进程会阻塞直到临界区内的进程退出才能进入临界区，被临界区保护起来的资源就是临界资源
 *  临界区的设计应该注意纯度和长度，非临界区代码最好不要写在临界区内增长锁的时间，但是同时也要避免频繁地加锁导致并行度降低。
 * 解决竞争条件的4个原则
 *  1.同一时刻只能有一个进程进入临界区
 *  2.不得对cpu的数量和速度做限制
 *  3.处于非临界区的进程不得阻塞处于临界区内的进程
 *  4.进程不得无限制地等待进入临界区
 */

//cpu忙等待型的解决方案
/**
 * 屏蔽中断
 *  （1）进程在进入临界区时，发出硬件指令通知cpu屏蔽包括时钟中断在内的一切中断。只要cpu不响应中断请求,则不会启动进程调度程序
 * 缺点：
 *  （1）把屏蔽中断交给用户程序明显是不合理的,如果一个用户程序在进入临界区时关闭了中断,离开临界区时忘记开启，那么整个cpu都无法工作了
 *   (2) 对于单核处理器，屏蔽中断可以防止进程在临界区内发生调度导致数据的不一致性，但是对于多核处理器，无法解决这个问题
 * 结论:
 *   违背了第二个原则,不得对cpu的数量和速度做出限制，因此不是用户层面解决竞争条件的好方法(注意这里讲的是"不是在用户层面解决竞态的好方法")
 */

/**
 * 锁变量
 *   取一个共享变量lock做锁，lock=0时说明当前临界区可以访问，lock=1时说明当前临界区拒绝访问。使用while(lock != 0);等待直到获得锁
 *   进程进入临界区时把lock设置成1，离开临界区时把lock重新设置为0
 * 存在问题
 *   （1）假设存在进程A和B，进程A检测到lock=0,即将进行后续逻辑，此时刚好发生了进程调度，进程B代替进程A在cpu上运行。进程B检测到lock=0，并把lock设置为1后进入临界区
 *   重新调度到进程A，进程A继续之前的逻辑,把lock设置成1后进入临界区。此时有两个进程同时进入了临界区，这就违背了4原则中的第一个原则。
 *   （2）可能有些同学会说,在进入临界区之前我在检查一遍lock,如果lock=1则不进入临界区不就好了吗？double check确实可以一定程度上减少竞态风险。
 *   但这属于治标不治本，如果进程的调度是发生在第二次check，那么上述存在的问题仍然会发生
 * 结论
 *   之所以锁变量会违背第一原则是因为测试锁并完成加锁的操作不是一个原子性操作，这也提醒了我在设计临界区时需要注意，不能使用非原子操作去设置锁变量。
 *   以及double check也可以减少一些性能上的开销(虽然无法最终解决问题)
 * 代码如下
 */
#define UNLOCK 0
#define LOCK 1
int lock = UNLOCK;
int _count = 0;
_Noreturn void* get_lock_for_progress_a() {
    while (1) {
        while (LOCK == lock);
        //临界区开始
        lock = LOCK;
        printf("current progress=a,enter region\n");
        ++_count;
        //临界区域结束
        lock = UNLOCK;
        //非临界区代码开始
        printf("current progress=a,leave region,count=%d\n",_count);
    }
}
_Noreturn void* get_lock_for_progress_b() {
    while (1) {
        while (LOCK == lock);
        lock = LOCK;
        printf("current progress=b,enter region\n");
        //临界区开始
        ++_count;
        //临界区域结束
        lock = UNLOCK;
        //非临界区代码开始
        printf("current progress=b,leave region,count=%d\n",_count);
    }
}

_Noreturn void run_get_lock_demo() {
    pthread_t progress_a,progress_b;
    pthread_create(&progress_a,NULL,get_lock_for_progress_a,NULL);
    pthread_create(&progress_b,NULL,get_lock_for_progress_b,NULL);
    while (1); //主线程阻塞住,避免进程退出
}

/**
 *  严格轮转法(又叫自旋)
 *    参与对象
 *      两个进程,暂定为A和B
 *  代码实现如下,输出如下
        current progress=a,enter region
        current progress=a,leave region
        current progress=b,enter region
        current progress=b,leave region
        current progress=a,enter region
        current progress=a,leave region
        current progress=b,enter region
        current progress=b,leave region
        current progress=a,enter region
        current progress=a,leave region
        current progress=b,enter region
        current progress=b,leave region
        current progress=a,enter region
        current progress=a,leave region
        current progress=b,enter region
        current progress=b,leave region
        current progress=a,enter region
        current progress=a,leave region
        current progress=b,enter region
        current progress=b,leave region
        current progress=a,enter region
        current progress=a,leave region
        current progress=b,enter region
        current progress=b,leave region
 *
 * 结论：
 *  可以看到都是进程a和进程b的进入和退出临界区都是按照严格轮转的模式的（a->b->a->b->a->b）,不同于锁(哪个进程获取到锁就可以执行)
 *  解锁操作并不需要在循环的最后才执行，只要退出临界区就可以马上解锁，缩小临界区的长度可以提升程序的并行度
*  缺点:
 *  这种解法存在一个致命性的缺点。即违反了四个原则中的第三点,处于非临界区的进程不得阻塞处于临界区内的进程
 *  举个栗子
 *      1.进程a首先进入临界区,把turn设置成1后退出临界区,执行非临界区的代码(假设a的非临界区的代码逻辑很复杂,执行很慢)
 *      2.进程b其后进入临界区，把turn设置成0后退出临界区，执行非临界区代码(假设b的非临界区代码很简单,执行很快)
 *      3.由于进程a还在非临界区内,所以turn一直等于0,此时进程b就需要等待turn被设置成1方可进入临界区。
 *      4.得出结论,处于非临界区的进程a阻塞了想要进入临界区的进程b,违背了第三原则(处于非临界区的进程不得阻塞处于临界区的进程)
 *      5.自旋使得两个进程都会占用cpu时间(忙等待),除非等待的时间很短,否则不建议使用该解法
*/
int turn = 0; //轮转控制
int count = 0; //临界资源
#define PROGRESS_A_NUMBER  0 //进程a的逻辑编号
#define PROGRESS_B_NUMBER  1 //进程b的逻辑编号

_Noreturn void* spain_lock_for_progress_a() {
    while (1) {
        while (turn != PROGRESS_A_NUMBER); //等待直到轮转到当前进程(进程A)
        printf("current progress=a,enter region\n");
        //临界区开始
        ++count;
        //临界区域结束
        turn = PROGRESS_B_NUMBER;
        //非临界区代码开始
        printf("current progress=a,leave region,count=%d\n",count);
        //非临界区代码结束
    }
}

_Noreturn void* spain_lock_for_progress_b() {
    while (1) {
        while (turn != PROGRESS_B_NUMBER); //等待直到轮转到当前进程(进程b)
        printf("current progress=b,enter region\n");
        //临界区开始
        ++count;
        //临界区域结束
        turn = PROGRESS_A_NUMBER;
        //非临界区代码开始
        printf("current progress=b,leave region,count=%d\n",count);
        //非临界区代码结束
    }
}

void run_spain_lock_demo() {
    pthread_t progress_a,progress_b;
    pthread_create(&progress_a,NULL,spain_lock_for_progress_a,NULL);
    pthread_create(&progress_b,NULL,spain_lock_for_progress_b,NULL);
    while (1); //主线程阻塞住,避免进程退出
    //sleep(3); //成功返回0,或者返回剩余的要睡眠的秒数(被signal中断后,说明这属于一个慢系统调用,signal是模拟硬件中断的软件机制,注册的信号回调执行后会从慢系统调用处返回(同理还有accept,write,read等)),
}


#define true 1
#define false 0
static int interest[] = {false,false};
/**
 * 分几种种情况
 *   (1)
 *      1.进程a执行interest[0] = true
 *      2.进程b执行interest[1] = true
 *      3.进程a执行 turn = 0
 *      4.进程b执行 turn = 1
 *      5.此时turn = 1,interest数组均为true
 *      6.所以进程b阻塞于while循环 while(turn == 1 && true == interest[0])
 *      7.进程a进入临界区,之后调用peterson_leave_region,把interest[0]设置成false
 *      8.进程b破坏循环条件,也进入了缓冲区
 *   (2)
 *      1.进程a执行interest[0] = true
 *      2.进程a执行 turn = 0
 *      3.进程b执行interest[1] = true
 *      4.进程b执行 turn = 1
 *      5.所以进程b阻塞于while循环 while(turn == 1 && true == interest[0])
 *      6.同上
 *  (3)
 *      1.进程a执行interest[0] = true
 *      2.进程b执行interest[1] = true
 *      3.进程b执行turn = 1
 *      4.同上
 *  (4)
 *      1.进程b执行interest[1] = true
 *      2.进程b执行turn = 1
 *      3.进程a执行interest[0] = true
 *      4.进程a执行turn = 0
 *      5.此时turn=0,interest数组均为true
 *      6.所以进程a阻塞于while循环 while(turn == 0 && true == interest[1])
 *      7.同上
 *  结论：每个进程进入临界区时都会把各自的interest设置成true,同时会把全局变量turn设置成自己的progress_number
 *  所以后续进程都会阻塞住自己,直到上一个进程离开临界区(把上一个进程的interest设置成false)
 *  启示: 如果进程之间可以天然通过一些标识符隔离数据(比如本例中使用数组索引访问各自的元素),则可以在无锁的条件下解决竞态条件
 * @param progress_number
 */
void peterson_enter_region(int progress_number) {
    int other = 1-progress_number; //取得互相竞争的进程id
    interest[progress_number] = true;//每个进程享有自己的存储空间(使用索引区分数组),类似threadLocal,可以减少锁争用
    turn = progress_number;//把轮转设置为当前进程的number
    while (turn == progress_number && true == interest[other]);
}

void peterson_leave_region(int progress_number) {
    interest[progress_number] = false;
}

_Noreturn void* peterson_for_progress_a() {
    while (1) {
        peterson_enter_region(PROGRESS_A_NUMBER);
        //临界区开始
        printf("current progress=a,enter region\n");
        ++count;
        printf("current progress=a,leave region,count=%d\n",count);
        //临界区结束
        peterson_leave_region(PROGRESS_A_NUMBER);
        //非临界区代码开始
    }
}

_Noreturn void* peterson_for_progress_b() {
    while (1) {
        peterson_enter_region(PROGRESS_B_NUMBER);
        //临界区开始
        printf("current progress=b,enter region\n");
        ++count;
        printf("current progress=b,leave region,count=%d\n",count);
        //临界区结束
        peterson_leave_region(PROGRESS_B_NUMBER);
        //非临界区代码开始
    }
}

/**
 * 运行结果
 *   current progress=a,enter region
     current progress=a,leave region,count=380393
     current progress=b,enter region
     current progress=b,leave region,count=380394
     current progress=a,enter region
     current progress=a,leave region,count=380395
     current progress=b,enter region
     current progress=b,leave region,count=380396
     current progress=a,enter region
     current progress=a,leave region,count=380397
     current progress=b,enter region
     current progress=b,leave region,count=380398
     current progress=a,enter region
     current progress=a,leave region,count=380399
     current progress=b,enter region
     current progress=b,leave region,count=380400
     可以看到,进程a,b分别进入了临界区
   优点：
     相比严格轮转法(自旋锁),运行顺序靠后的进程(最后一个覆盖变量turn的进程b)会因while循环陷入阻塞。
     但是无论前一个进程a的非临界区代码业务有多复杂,只要a进程离开了临界区(调用leave_region)。那么
     被while循环阻塞的进程b就会马上破坏循环条件进入临界区。在a阻塞于非临界区时,b会不断地获得进入临界区的资格。
     这就完美解决了竞态条件原则第三点的冲突,非临界区的进程不得阻塞位于临界区的进程
 */
_Noreturn void peterson_demo() {
    pthread_t progress_a,progress_b;
    pthread_create(&progress_a,NULL,peterson_for_progress_a,NULL);
    pthread_create(&progress_b,NULL,peterson_for_progress_b,NULL);
    while (1); //主线程阻塞住,避免进程退出
}


/**
    TSL <==> test and set lock
    使用汇编代码级别实现的原子性测试并加锁流程。过程如下
      1.在内存中有个共享内存字，我们约定为lock。当lock=0时可以进入临界区，lock=1时说明不可以进入临界区
      2.当cpu访问该内存字的时候，首先会锁住内存总线，防止其他cpu同时访问该内存字
      3.把该内存字的值复制到寄存器中，并把改内存字设置为1
      4.使用CMP指令判断该寄存器中的值是否为0，如果是则返回给上一层调用说明可以进入临界区，否则重新尝试测试并设置锁的操作[如果是用户态线程的实现,测试锁失败的时候就应该通过yield操作主动让出cpu给其他线程(避免内核线程阻塞)]
    汇编代码如下
      enter_region:
          TSL #REGISTER lock (把一个内存字复制到寄存器中,并把lock设置为1)
          CMP #REGISTER 0 (比较寄存器的值是否为0,为0可以进入临界区)
          RET (返回到上一层调用)
          JMP TST (不可以进入临界区则重新执行tsl)
      leave_region:
          MOVE lock 0 (设置内存字为0)
     TSL是通过汇编代码保障的原子性，cpu在访问内存字时会锁住内存总线以避免其他cpu竞争访问。配合屏蔽中断操作，可以在cpu层面
     避免竞条件的产生。一般屏蔽中断+tsl操作会用于信号量的up and down操作时保障原子性，因为其锁住的时间仅仅只是几条指令的执行时间。(比起进程等待临界区开启的时间很小)
     所以对性能的损耗并不会很大，这个应该和解决竞态条件的原则2--不得对cpu的数量和速度做出限制进行区分。

 */