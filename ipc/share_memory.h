//
// Created by bytedance on 2020/9/12.
//

#ifndef OS_SHARE_MEMORY_H
#define OS_SHARE_MEMORY_H
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include "string.h"

typedef int counter;

static void read_from_share_memory();
static void write_from_share_memory();
void run_share_memory_demo();
#endif //OS_SHARE_MEMORY_H
