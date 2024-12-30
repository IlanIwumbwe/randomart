#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <assert.h>

#define U64 __uint64_t

float randrange(float min, float max){
    assert(max > min);
    
    return min + (rand() / (float)RAND_MAX) * (max - min);
}

void usage(){
    printf("Usage: ./randomart [-d] _ [-s] _ [-t]\n");
    printf("Using default options:\n- current time seed\n- depth = 1\n-image render\n");
}

int set_depth(char* argv[], int flag_pos, int* depth){
    char* end;

    if(!strcmp(argv[flag_pos], "-d")){
        *depth = strtol(argv[flag_pos+1], &end, 10);
        return 0;
    } else {
        return -1;
    }
}

int set_seed(char* argv[], int flag_pos, U64* seed){
    char* end;

    if(!strcmp(argv[flag_pos], "-s")){
        *seed = strtoll(argv[flag_pos+1], &end, 10);
        return 0;
    } else {
        return -1;
    }
}

int set_test(char* argv[], int flag_pos, int* test){

    if(!strcmp(argv[flag_pos], "-t")){
        *test = 1;
        return 0;
    } else {
        return -1;
    }
}

#endif
