//
// Created by bytedance on 2020/9/13.
//

#ifndef OS_ANONYMOUS_PIPE_H
#define OS_ANONYMOUS_PIPE_H
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void read_from_anonymous_pipe(int read_pipe,char *r_buf,int r_buf_len); //从匿名管道中读取
void write_from_anonymous_pipe(int write_pipe,char *w_buf,int w_buf_len); //往匿名管道中写入
void run_anonymous_pipe_demo();

#endif //OS_ANONYMOUS_PIPE_H
