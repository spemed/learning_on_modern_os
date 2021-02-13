cmake_minimum_required(VERSION 3.16)
project(os C)

set(CMAKE_C_STANDARD 99)

add_executable(main.c pthread/0.hello_thread/hello_thread.c pthread/0.hello_thread/hello_thread.h data_race/cpu_busy.c data_race/cpu_busy.h data_race/block.c data_race/block.h ipc/share_memory.c ipc/share_memory.h main.c ipc/anonymous_pipe.c ipc/anonymous_pipe.h ipc/semaphore.c ipc/semaphore.h ipc/signal.c ipc/signal.h ipc/philosopher.c ipc/philosopher.h main.c)