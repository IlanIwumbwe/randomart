#include <stdio.h>
#include <time.h>
#include "../headers/render.h"
#include "../headers/interpreter.h"
#include "../headers/grammar.h"

void init(int seed){

    srand(seed);

    grammar();

    init_ast(20);
}

void build_ast(int depth){

    // build AST from grammar
    generate_ast(g.entry_point, fmin(depth, MAX_DEPTH));

    ast.size = ast.used; // set size of AST right after generating it

    print_ast_ln(ast.array_head);

    printf("nodes in AST: %ld\n", ast.size);

    /*
        In the worst case scenario, we need to create as many nodes as there are in the AST in order to evaluate it. This happens if there's no number nodes
        in the AST. This means that if we want no reallocations to happen during evaluation i.e we never run our of space in the dynamic array, we need to make 
        sure that the AST is stored with a capacity that's at least twice as big as the AST size. 

        5 added just to be extra extra safe
    */

    if((ast.capacity - ast.size) <  2 * (ast.size + 5)){
        reallocate_ast(2 * (ast.size + 5));
    }
}

void run(int depth, Run_mode mode){

    build_ast(fmin(depth, MAX_DEPTH));

    if(mode == RM_TEST){
        test_eval();
    } else if (mode == RM_RENDER){
        render_image();
    }
}

int main(int argc, char* argv[]){

    U64 seed = (U64)time(NULL); 
    int depth = 0;
    Run_mode mode = RM_RENDER;
    int error = 0;

    if(argc == 6){
        error += set_depth(argv, 1, &depth) + set_seed(argv, 3, &seed) + set_mode(argv, 5, &mode);

    } else if(argc == 5){
        error += set_depth(argv, 1, &depth) + set_seed(argv, 3, &seed);
        
    } else if(argc == 4){
        error += set_depth(argv, 1, &depth) + set_mode(argv, 3, &mode);

    } else if (argc == 3) {
        error+= set_depth(argv, 1, &depth);

    } else if (argc == 2){
        error = set_mode(argv, 1, &mode);
    }

    if(error){
        usage();
    }

    init(seed);

    run(depth, mode);

    free_ast();
    free_grammar();

    return 0;
}

