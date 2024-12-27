#ifndef DEFS_H
#define DEFS_H

#include <math.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../headers/stb_image_write.h"

#define IMAGE_SIZE 256

typedef struct {
    float r;
    float g;
    float b;
} Colour;

typedef struct {
    char r;
    char g;
    char b;
    char a;
} Pixel;

#endif
