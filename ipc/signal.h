//
// Created by bytedance on 2020/11/2.
//

#ifndef OS_SIGNAL_H
#define OS_SIGNAL_H
#include <signal.h>
typedef void sigfunc(int) ;
static sigfunc *signal_setter(int signo,sigfunc *func);
void signal_show();
#endif //OS_SIGNAL_H
