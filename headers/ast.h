#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "utils.h"

#define U64 __uint64_t

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
    int line;
    char* file;

    float prob;
};

typedef struct{
    Node* array;
    size_t used;
    size_t capacity;

    Node* array_head;
    size_t size; // size of AST after initial generation
} Ast;

extern Ast ast;

void init_ast(Ast* ast, size_t capacity){

    if(capacity <= 0){
        printf("[ERROR] Cannot initialise dynamic array with capacity of %ld!\n", capacity);
        exit(-1);
    } else {
        ast->array = (Node*) malloc(sizeof(Node) * capacity);
        
        if(ast->array == NULL){
            printf("[ERROR] Memory allocation of %ld elements failed!\n", capacity);
            exit(-1);
        }

        ast->capacity = capacity;
        ast->used = 0;
    }
}

void free_ast(Ast* ast){
    free(ast);
    printf("Freed ast memory\n");
}

/// @brief Reset `array_head` and `used` pointers to point to the root of the AST that was generated. This is required to prepare new evaluation       
/// @param ast 
void find_ast_root(Ast* ast){
    ast->array_head = ast->array + ast->size - 1; // reset head pointer to top of AST to setup re-evaluation
    ast->used = ast->size; // reset used counter to overwrite created nodes during previous evaluation
}

void reallocate_node_pointers(Node n, Node* old_node_loc, Node* new_node_loc){

    if(old_node_loc != new_node_loc){

        if((n.nk == NK_ADD) || (n.nk == NK_MULT)){
            unsigned long lhs_offset = old_node_loc - n.as.binop.lhs;
            unsigned long rhs_offset = old_node_loc - n.as.binop.rhs;

            assert(lhs_offset > 0);
            assert(rhs_offset > 0);

            new_node_loc->as.binop.lhs = new_node_loc - lhs_offset;
            new_node_loc->as.binop.rhs = new_node_loc - rhs_offset;

            #ifdef DEBUG
            printf("reallocating for node in file %s at line %d\n", n.file, n.line);

            printf("old lhs loc %lx ", (U64)n.as.binop.lhs);
            printf("old rhs loc %lx ", (U64)n.as.binop.rhs);
            printf("\n");
            printf("new lhs loc %lx ", (U64)new_node_loc->as.binop.lhs);
            printf("new rhs loc %lx \n", (U64)new_node_loc->as.binop.rhs);
            #endif
        }

        if(n.nk == NK_TRIPLE){
            unsigned long first_offset = old_node_loc - n.as.triple.first;
            unsigned long second_offset = old_node_loc - n.as.triple.second;
            unsigned long third_offset = old_node_loc - n.as.triple.third;

            assert(first_offset > 0);
            assert(second_offset > 0);
            assert(third_offset > 0);

            new_node_loc->as.triple.first = new_node_loc - first_offset;
            new_node_loc->as.triple.second = new_node_loc - second_offset;
            new_node_loc->as.triple.third = new_node_loc - third_offset;

            #ifdef DEBUG
            printf("reallocating for node in file %s at line %d\n", n.file, n.line);

            printf("old first loc %lx ", (U64)n.as.triple.first);
            printf("old second loc %lx ", (U64)n.as.triple.second);
            printf("old third loc %lx ", (U64)n.as.triple.third);
            printf("\n");
            printf("new first loc %lx ", (U64)new_node_loc->as.triple.first);
            printf("new second loc %lx ", (U64)new_node_loc->as.triple.second);
            printf("new third loc %lx \n", (U64)new_node_loc->as.triple.third);
            #endif
        }

    }
}

/// @brief Move ast node array to a new mem location
/// @param new_cap New array capacity
void reallocate_ast(Ast* ast, size_t new_cap){
    ast->capacity = new_cap;

    Node* nn = (Node*)realloc(ast->array, sizeof(Node) * ast->capacity);
    
    if(nn == NULL){
        printf("[ERROR] Memory reallocation of failed!\n");
        free_ast(ast);
        exit(-1);
    }

    // move pointers of nodes to point to the new memory locations if a reallocation happens during AST building
    for (size_t i = 0; i < ast->size; ++i){
        reallocate_node_pointers(ast->array[i], ast->array+i, nn+i);
    }

    ast->array = nn; // move array pointer
}

Node* add_node_to_ast(Ast* ast, Node node){

    if(ast->used >= ast->capacity){
        reallocate_ast(ast, 2 * ast->capacity);
    }

    ast->array[ast->used++] = node;
    
    assert(ast->used != 0);

    ast->array_head = ast->array + ast->used - 1; // point to node that just got added

    return ast->array_head;  // return pointer to node that just got added
}

Node* node_number_loc(float n, int line, char* file){
    Node node;
    node.nk = NK_NUMBER;

    node.as.number = n;
    node.file = file;
    node.line = line;

    return add_node_to_ast(&ast, node); 
}

Node* node_x_loc(int line, char* file){
    Node node;
    node.nk = NK_X;

    node.file = file;
    node.line = line;

    return add_node_to_ast(&ast, node);
}

Node* node_y_loc(int line, char* file){
    Node node;
    node.nk = NK_Y;

    node.file = file;
    node.line = line;
    
    return add_node_to_ast(&ast, node);
}

Node* node_add_loc(Node* lhs, Node* rhs, int line, char* file){
    Node node;
    node.nk = NK_ADD;

    node.as.binop.lhs = lhs;
    node.as.binop.rhs = rhs;
    node.file = file;
    node.line = line;

    return add_node_to_ast(&ast, node);
}

Node* node_mult_loc(Node* lhs, Node* rhs, int line, char* file){
    Node node;
    node.nk = NK_MULT;

    node.as.binop.lhs = lhs;
    node.as.binop.rhs = rhs;
    node.file = file;
    node.line = line;

    return add_node_to_ast(&ast, node);
}

Node* node_triple_loc(Node* first, Node* second, Node* third, int line, char* file){
    Node node;
    node.nk = NK_TRIPLE;

    node.as.triple.first = first;
    node.as.triple.second = second;
    node.as.triple.third = third;
    node.file = file;
    node.line = line;

    return add_node_to_ast(&ast, node);
}

#define node_add(lhs, rhs) node_add_loc(lhs, rhs, __LINE__, __FILE__)
#define node_number(n) node_number_loc(n, __LINE__, __FILE__)
#define node_x node_x_loc(__LINE__, __FILE__)
#define node_y node_y_loc(__LINE__, __FILE__)
#define node_triple(first, second, third) node_triple_loc(first, second, third, __LINE__, __FILE__)
#define node_mult(lhs, rhs) node_mult_loc(lhs, rhs, __LINE__, __FILE__)

Node* node_A(){

    float branch_prob = randrange(0, 1);

    if(branch_prob < 1.0/3.0){
        return node_number(randrange(-1, 1));

    } else if (branch_prob < 2.0/3.0){
        return node_x;

    } else {
        return node_y;
    }
}

Node* node_C(int depth){

    float branch_prob = randrange(0, 1);

    if((branch_prob < 1.0/4.0) || (depth == 0)){
        return node_A();

    } else if (branch_prob < 5.0/8.0){
        return node_add(node_C(depth - 1), node_C(depth - 1));

    } else {
        return node_mult(node_C(depth - 1), node_C(depth - 1));
    }

}

/// @brief Print the AST
/// @param n 
void print_ast(Node* n){

    switch(n->nk){
        case NK_X: 
            printf("x"); break;

        case NK_Y:
            printf("y"); break;

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

void build_ast(int depth){
    init_ast(&ast, 20);

    // build AST. A and C nodes will follow the grammar rules to generate the AST
    node_triple(
        node_C(depth), 
        node_C(depth),
        node_C(depth)
    );

    ast.size = ast.used; // set size of AST right after generating it

    print_ast_ln(ast.array_head);

    printf("nodes in AST: %ld\n", ast.size);

    // evaluation needs at least as many nodes as AST size for evaluation, reallocate AST to a new location with twice the capacity
    // this ensures that the AST doesn't have to be reallocated during evaluation, which makes things a bit more tricky to deal with
    // 2 times capacity is actually an overestimate of additional memory needed but that's a good thing
    reallocate_ast(&ast, 2 * ast.capacity);
}

#endif