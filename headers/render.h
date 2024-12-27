#ifndef RENDER_H
#define RENDER_H

#include "defs.h"

/// @brief Return rgb value at that pixel [-1, 1]
/// @param x [-1, 1] 
/// @param y [-1, 1]
/// @return 
Colour greyscale(float x, float y){
    Colour c = {x, x, x};

    return c;
}

Colour comp(float x, float y){

    if(x*y >= 0.0){
        return (Colour) {x, y, 1.0};
    } else {
        return (Colour) {fmod(x, y), fmod(x, y), fmod(x, y)};
    }
}

Colour other(float x, float y){
    return (Colour){x, x, y};
}

void render_image(Colour (*f)(float x, float y)){
    float f_x, f_y;
    Pixel canvas[IMAGE_SIZE][IMAGE_SIZE];

    for(int int_y = 0; int_y < IMAGE_SIZE; ++int_y){
        for(int int_x = 0; int_x < IMAGE_SIZE; ++int_x){
            // map pixel coordinates to [-1, 1]
            f_x = ((float)int_x / (float)IMAGE_SIZE) * 2.0 - 1.0;
            f_y = ((float)int_y / (float)IMAGE_SIZE) * 2.0 - 1.0;

            Colour c = f(f_x, f_y);

            canvas[int_y][int_x].r = ((c.r+1)/2.0) * 255;
            canvas[int_y][int_x].g = ((c.g+1)/2.0) * 255;
            canvas[int_y][int_x].b = ((c.b+1)/2.0) * 255;
            canvas[int_y][int_x].a = 255;
        }
    }

    if(!stbi_write_png("randomart.png", IMAGE_SIZE, IMAGE_SIZE, 4, *canvas, sizeof(Pixel) * IMAGE_SIZE)){
        printf("[ERROR] could not write image\n");
    }  
}


#endif