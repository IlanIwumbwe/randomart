#include <stdio.h>
#include <time.h>
#include "../headers/render.h"
#include "../headers/interpreter.h"

Ast ast;

int main(int argc, char* argv[]){

    U64 seed = (U64)time(NULL); 
    int depth = 1;
    int test = 0;
    int error = 0;

    if(argc == 6){
        error += set_depth(argv, 1, &depth) + set_seed(argv, 3, &seed) + set_test(argv, 5, &test);

    } else if(argc == 5){
        error += set_depth(argv, 1, &depth) + set_seed(argv, 3, &seed);
        
    } else if(argc == 4){
        error += set_depth(argv, 1, &depth) + set_test(argv, 3, &test);

    } else if (argc == 3) {
        error+= set_depth(argv, 1, &depth);

    } else if (argc == 2){
        error = set_test(argv, 1, &test);
    }

    if(error){
        usage();
    }

    srand(seed);

    build_ast(fmin(depth, MAX_DEPTH));

    if(test){
        test_eval(&ast);
    } else {
        render_image(&ast);
    }

    return 0;
}

