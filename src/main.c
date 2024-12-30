#include <stdio.h>
#include <time.h>
#include "../headers/render.h"
#include "../headers/interpreter.h"

Ast ast;

void usage(){
    printf("Usage: ./randomart -d [depth] -s [seed]\n");
}

int main(int argc, char* argv[]){

    U64 seed = (U64)time(NULL); 
    int depth = 1;
    char* end;

    if(argc == 5){

        if(!strcmp(argv[1], "-d")){
            depth = strtol(argv[2], &end, 10);
        } else {
            usage();
        }

        if(!strcmp(argv[3], "-s")){
            seed = strtoll(argv[4], &end, 10);
        } else {
            usage();
        }
        
    } else if (argc == 3) {

        if(!strcmp(argv[1], "-d")){
            depth = strtol(argv[2], &end, 10);
        } else {
            usage();
        }

    } else if (argc != 1){
        usage();
    }

    srand(seed);

    build_ast(depth);

    render_image(&ast);

    //print_ast_ln(eval(&ast, 10, 3));

    return 0;
}

