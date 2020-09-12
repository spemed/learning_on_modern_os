#include <stdio.h>
#include "pthread/0.hello_thread/hello_thread.h"
#include "data_race/cpu_busy.h"
#include "ipc/share_memory.h"

int main() {
    run_share_memory_demo();
    return 0;
}
