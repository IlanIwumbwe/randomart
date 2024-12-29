#include <stdio.h>
#include "../headers/render.h"
#include "../headers/interpreter.h"

Ast ast;

int main(){

    init_ast(&ast, 20);

    node_triple(
        node_add(
            node_y, 
            node_add(
                node_x, 
                node_x
            )
        ),
        node_add(
            node_x,
            node_add(
                node_x,
                node_y
            )
        ),
        node_x
    );

    ast.size = ast.used; // set size of AST right after generating it

    printf("nodes: %ld\n", ast.size);

    // render_image(&ast);

    print_ast_ln(eval(&ast, 1.5, 2.5));
    // print_ast_ln(eval(&ast, 2, 2));

    return 0;
}

