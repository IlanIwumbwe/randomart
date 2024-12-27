#ifndef NODE_H
#define NODE_H

#include <stddef.h>

typedef enum {
    NK_X,
    NK_Y,
    NK_NUMBER,
    NK_ADD,
    NK_MULT,
    NK_TRIPLE,
} Node_kind;

typedef struct s_Node Node;

typedef struct{
    Node* rhs;
    Node* lhs;    
} Binop;

typedef struct{
    Node* first;
    Node* second; 
    Node* third;  
} Triple;

typedef union{
    Binop binop;
    Triple triple;
    float number;
} Node_as;

struct s_Node {
    Node_kind nk;
    Node_as as;
};

typedef struct{
    Node* n;
    size_t used;
    size_t capacity;
    Node* head;
} Ast;

void init_ast(Ast* ast, size_t capacity){

    if(capacity <= 0){
        printf("[ERROR] Cannot initialise dynamic array with capacity of %ld!\n", capacity);
        exit(-1);
    } else {
        ast->n = malloc(sizeof(Node) * capacity);
        
        if(ast == NULL){
            printf("[ERROR] Memory allocation of %ld bytes failed!\n", capacity);
            exit(-1);
        }

        ast->capacity = capacity;
        ast->used = 0;

        printf("Allocated %ld elements for dynamic array\n", ast->capacity);
    }
}

void free_ast(Ast* ast){
    free(ast);
    printf("Freed ast memory\n");
}

Node* add_node_to_ast(Ast* ast, Node node){

    if(ast->used >= ast->capacity){
        printf("used %ld cap %ld\n", ast->used, ast->capacity);
        ast->capacity = 2 * ast->capacity;
        
        Node* nn = realloc(ast->n, sizeof(Node) * ast->capacity);

        if(nn == NULL){
            printf("[ERROR] Memory reallocation of failed!\n");
            free_ast(ast);
            exit(-1);
        }

        ast->n = nn;
    }

    ast->n[ast->used++] = node;

    assert(ast->used != 0);

    ast->head = ast->n + ast->used - 1;

    return ast->head;
}

#endif

