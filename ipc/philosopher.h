//
// Created by bytedance on 2021/2/12.
//

#ifndef OS_PHILOSOPHER_H
//哲学家的数目
#define PEOPLE 5
#define LEFT(i) i
#define RIGHT(i) ((i+1)%(PEOPLE))

#define THINKING 0
#define EATING 1
#define HUNGRY 2

void run();
#define OS_PHILOSOPHER_H

#endif //OS_PHILOSOPHER_H
