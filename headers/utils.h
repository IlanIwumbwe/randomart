#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define U64 __uint64_t
#define MAX_DEPTH 50
#define INPUT_SIZE 20000

typedef enum{
    RM_RENDER,
    RM_TEST,
    RM_PRINT
} Run_mode;

float randrange(float min, float max){
    assert(max > min);
    
    return min + (rand() / (float)RAND_MAX) * (max - min);
}

float clamp(float x, float min, float max){
    if(x <= min){return min;}
    if(x > max){return max;} else {return x;}
}  

static int get_input(char* buffer){
    printf("> ");
    memset(buffer, 0, INPUT_SIZE);
    fflush(stdout);

    if(fgets(buffer, INPUT_SIZE, stdin) == NULL){
        return -1;
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';

    return 0;
}

#endif
