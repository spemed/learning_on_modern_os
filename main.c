#include <stdio.h>
#include "pthread/0.hello_thread/hello_thread.h"
#include "data_race/cpu_busy.h"
int main() {
    peterson_demo();
    return 0;
}
