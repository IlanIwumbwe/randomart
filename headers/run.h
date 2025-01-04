#ifndef RUN_H
#define RUN_H

#include "utils.h"
#include "grammar.h"
#include "parser.h"
#include "render.h"
#include "interpreter.h"

void init(){

    grammar();
    print_grammar();
    init_ast(20);
}

void run(){
    U64 seed; 
    int depth;
    int seed_set = 0;
    Run_mode mode;

    init();

    char command[INPUT_SIZE];
    char* end;

    while(!get_input(command)){

        if(!seed_set) seed = (U64)time(NULL);
        else {seed_set = 0;}

        reset_ast();

        if (!strncmp(command, "quit", 4)){
            break;
        } else if(!strncmp(command, "depth", 5)){
            depth = fmin(MAX_DEPTH, strtol(command+6, &end, 10));
            continue;
        } else if (!strncmp(command, "test", 4)){
            mode = RM_TEST;
            continue;
        } else if (!strncmp(command, "seed", 4)){
            seed = strtoll(command+5, &end, 10);
            seed_set = 1;
            continue;
        } else if (!strncmp(command, "render", 6)){
            mode = RM_RENDER;
            continue;
        } else if (parse(command) != 0){
            srand(seed);

            generate_ast(g.entry_point, depth);
        
            print_ast_ln(ast.array_head);
        }

        ast.size = ast.used; // set size of AST right after generating it
        reallocate_ast_after_build();

        if(mode == RM_TEST){
            printf("Testing AST on random point.....\n");
            test_eval();
            printf("\n");

        } else if (mode == RM_RENDER){
            printf("Rendering image.....\n");
            render_image();
            printf("\n");
        }

    }
}

#endif

