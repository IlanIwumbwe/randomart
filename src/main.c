#include <stdio.h>
#include "../headers/render.h"
#include "../headers/interpreter.h"

Ast ast;

int main(){

    build_ast();
    
    render_image(&ast);

    //print_ast_ln(eval(&ast, 1.5, 2.5));

    return 0;
}

