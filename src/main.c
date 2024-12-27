#include <stdio.h>
#include "../headers/render.h"
#include "../headers/node.h"

Ast ast;

void print_ast(Node* n){

    switch(n->nk){
        case NK_X: 
            printf("x "); break;

        case NK_Y:
            printf("y "); break;

        case NK_ADD:
            printf("add(");
            print_ast(n->as.binop.lhs);
            printf(", ");
            print_ast(n->as.binop.rhs);
            printf(")");
            break;

        case NK_MULT:
            printf("mult(");
            print_ast(n->as.binop.lhs);
            printf(", ");
            print_ast(n->as.binop.rhs);
            printf(")");
            break;

        case NK_TRIPLE:
            printf("triple(");
            print_ast(n->as.triple.first);
            printf(",");
            print_ast(n->as.triple.second);
            printf(",");
            print_ast(n->as.triple.third);
            printf(")");
            break;

        case NK_NUMBER:
            printf("%f", n->as.number); break;

        default:
            printf("Should not be able to reach this in print ast!\n");
            exit(-1);
    }
}

#define print_ast_ln(node) (print_ast(node), printf("\n"))

Node* node_number(float n){
    Node node;
    node.nk = NK_NUMBER;
    node.as.number = n;

    return add_node_to_ast(&ast, node);
}

Node* node_add(Node* lhs, Node* rhs){
    Node node;
    node.nk = NK_ADD;
    node.as.binop.lhs = lhs;
    node.as.binop.rhs = rhs;

    return add_node_to_ast(&ast, node);
}

Node* node_mult(Node* lhs, Node* rhs){
    Node node;
    node.nk = NK_MULT;
    node.as.binop.lhs = lhs;
    node.as.binop.rhs = rhs;

    return add_node_to_ast(&ast, node);
}

Node* node_triple(Node* first, Node* second, Node* third){
    Node node;
    node.nk = NK_TRIPLE;
    node.as.triple.first = first;
    node.as.triple.second = second;
    node.as.triple.third = third;

    return add_node_to_ast(&ast, node);
}

int main(){

    render_image(&greyscale);

    init_ast(&ast, 20);

    printf("%ld\n", ast.capacity);

    node_triple(node_number(0.5), node_number(0.5), node_number(0.5));

    print_ast_ln(ast.head);
    
    return 0;
}

