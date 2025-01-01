#ifndef RENDER_H
#define RENDER_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <math.h>
#include "interpreter.h"

#define IMAGE_SIZE 256

typedef struct {
    char r;
    char g;
    char b;
    char a;
} Pixel;

int render_image(){
    float f_x, f_y;
    Pixel canvas[IMAGE_SIZE][IMAGE_SIZE];

    for(int int_y = 0; int_y < IMAGE_SIZE; ++int_y){
        for(int int_x = 0; int_x < IMAGE_SIZE; ++int_x){
            // map pixel coordinates to [-1, 1]
            f_x = ((float)int_x / (float)IMAGE_SIZE) * 2.0 - 1.0;
            f_y = ((float)int_y / (float)IMAGE_SIZE) * 2.0 - 1.0;

            Node* res = eval(f_x, f_y); // sample function built from AST

            if(res == NULL){
                printf("[%s] AST is invalid! Cannot evaluate it\n", __FILE__);
                return -1;
            }

            if(res->nk != NK_E){
                printf("[FILE %s] Final output from AST must be E! AST head added at line %d does not evaluate to that\n", res->file, res->line);
                return -1;
            }

            canvas[int_y][int_x].r = (res->as.triple.first->as.number+1)/2.0 * 255;
            canvas[int_y][int_x].g = (res->as.triple.second->as.number+1)/2.0 * 255;
            canvas[int_y][int_x].b = (res->as.triple.third->as.number+1)/2.0 * 255;
            canvas[int_y][int_x].a = 255;
        }
    }

    if(!stbi_write_png("randomart.png", IMAGE_SIZE, IMAGE_SIZE, 4, *canvas, sizeof(Pixel) * IMAGE_SIZE)){
        printf("[ERROR] could not write image\n");
        return -1;
    }

    return 0;  
}


#endif