#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <assert.h>

float randrange(float min, float max){
    assert(max > min);
    
    return min + (rand() / (float)RAND_MAX) * (max - min);
}


#endif
