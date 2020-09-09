//
// Created by bytedance on 2020/9/6.
// 基于cpu忙等待解决静态条件的方法
//

#ifndef OS_CPU_BUSY_H
#define OS_CPU_BUSY_H
#include <stdio.h>

/**
 * 严格轮换的自旋锁
 * @return
 */
_Noreturn void* spain_lock_for_progress_a();
_Noreturn void* spain_lock_for_progress_b();
_Noreturn void run_spain_lock_demo();

/**
 * 共享锁变量法
 * @return
 */
_Noreturn void* get_lock_for_progress_a();
_Noreturn void* get_lock_for_progress_b();
_Noreturn void run_get_lock_demo();

/**
 * peterson法
 * @return
 */

 void peterson_enter_region(int ); //进入临界区
 void peterson_leave_region(int ); //离开临界区
_Noreturn void* peterson_for_progress_a();
_Noreturn void* peterson_for_progress_b();
_Noreturn void peterson_demo();

#endif //OS_CPU_BUSY_H
