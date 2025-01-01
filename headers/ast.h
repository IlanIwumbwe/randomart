#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "utils.h"

#define U64 __uint64_t
#define set_bit(i) (1UL << i)

typedef enum {
    NK_X = set_bit(0), 
    NK_Y = set_bit(1),
    NK_NUMBER = set_bit(2),
    
    NK_SIN = set_bit(3),
    NK_COS = set_bit(4),
    NK_EXP = set_bit(5),

    NK_ADD = set_bit(6),
    NK_MULT = set_bit(7),
    NK_MOD = set_bit(8),
    NK_DIV = set_bit(9),
    NK_GTEQ = set_bit(10),

    NK_E = set_bit(11),
    NK_IF_THEN_ELSE = set_bit(12)
} Node_kind;

#define NK_UNOP (NK_SIN | NK_COS | NK_EXP)
#define NK_BINOP (NK_ADD | NK_MULT | NK_MOD | NK_DIV | NK_GTEQ)
#define NK_TRIPLE (NK_E | NK_IF_THEN_ELSE)

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
    Node* unop;
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

Ast ast = {0};

void init_ast(size_t capacity){

    if(capacity <= 0){
        printf("[ERROR] Cannot initialise dynamic array with capacity of %ld!\n", capacity);
        exit(-1);
    } else {
        ast.array = (Node*) malloc(sizeof(Node) * capacity);
        
        if(ast.array == NULL){
            printf("[ERROR] Memory allocation of %ld elements failed!\n", capacity);
            exit(-1);
        }

        ast.capacity = capacity;
        ast.used = 0;
    }
}

void free_ast(){
    free(ast.array);
    printf("Freed ast memory\n");
}

/// @brief Reset `array_head` and `used` pointers to point to the root of the AST that was generated. This is required to prepare new evaluation       
/// @param ast 
void find_ast_root(){
    ast.array_head = ast.array + ast.size - 1; // reset head pointer to top of AST to setup re-evaluation
    ast.used = ast.size; // reset used counter to overwrite created nodes during previous evaluation
}

void reallocate_node_pointers(Node n, Node* old_node_loc, Node* new_node_loc){

    if(old_node_loc != new_node_loc){

        if(n.nk & NK_UNOP){
            unsigned long arg_offset = old_node_loc - n.as.unop;

            assert(arg_offset > 0);

            new_node_loc->as.unop = new_node_loc - arg_offset;
        }

        if(n.nk & NK_BINOP){
            unsigned long lhs_offset = old_node_loc - n.as.binop.lhs;
            unsigned long rhs_offset = old_node_loc - n.as.binop.rhs;

            assert(lhs_offset > 0);
            assert(rhs_offset > 0);

            new_node_loc->as.binop.lhs = new_node_loc - lhs_offset;
            new_node_loc->as.binop.rhs = new_node_loc - rhs_offset;
        }

        if(n.nk & NK_TRIPLE){
            unsigned long first_offset = old_node_loc - n.as.triple.first;
            unsigned long second_offset = old_node_loc - n.as.triple.second;
            unsigned long third_offset = old_node_loc - n.as.triple.third;

            assert(first_offset > 0);
            assert(second_offset > 0);
            assert(third_offset > 0);

            new_node_loc->as.triple.first = new_node_loc - first_offset;
            new_node_loc->as.triple.second = new_node_loc - second_offset;
            new_node_loc->as.triple.third = new_node_loc - third_offset;
        }

    }
}

void free_grammar();

/// @brief Move ast node array to a new mem location
/// @param new_cap New array capacity
void reallocate_ast(size_t new_cap){
    ast.capacity = new_cap;

    Node* nn = (Node*)realloc(ast.array, sizeof(Node) * ast.capacity);
    
    if(nn == NULL){
        printf("[ERROR] Memory reallocation of failed!\n");
        free_ast(ast);
        free_grammar();
        exit(-1);
    }

    // move pointers of nodes to point to the new memory locations if a reallocation happens during AST building
    for (size_t i = 0; i < ast.size; ++i){
        reallocate_node_pointers(ast.array[i], ast.array+i, nn+i);
    }

    ast.array = nn; // move array pointer
}

Node* add_node_to_ast(Node node){

    if(ast.used >= ast.capacity){
        reallocate_ast(2 * ast.capacity);
    }

    ast.array[ast.used++] = node;
    
    assert(ast.used != 0);

    ast.array_head = ast.array + ast.used - 1; // point to node that just got added

    return ast.array_head;  // return pointer to node that just got added
}

Node* node_number_loc(float n, int line, char* file){
    Node node;
    node.nk = NK_NUMBER;

    node.as.number = n;
    node.file = file;
    node.line = line;

    return add_node_to_ast(node); 
}

Node* node_x_loc(int line, char* file){
    Node node;
    node.nk = NK_X;

    node.file = file;
    node.line = line;

    return add_node_to_ast(node);
}

Node* node_y_loc(int line, char* file){
    Node node;
    node.nk = NK_Y;

    node.file = file;
    node.line = line;
    
    return add_node_to_ast(node);
}

Node* node_unop_loc(Node_kind nk, Node* arg, int line, char* file){
    assert(nk & NK_UNOP);

    Node node;
    node.nk = nk;

    node.line = line;
    node.file = file;
    node.as.unop = arg;

    return add_node_to_ast(node);
}

Node* node_binop_loc(Node_kind nk, Node* lhs, Node* rhs, int line, char* file){
    assert(nk & NK_BINOP);

    Node node;
    node.nk = nk;

    node.as.binop.lhs = lhs;
    node.as.binop.rhs = rhs;
    node.file = file;
    node.line = line;

    return add_node_to_ast(node);
}

Node* node_triple_loc(Node_kind nk, Node* first, Node* second, Node* third, int line, char* file){
    assert(nk & NK_TRIPLE);
    
    Node node;
    node.nk = nk;

    node.as.triple.first = first;
    node.as.triple.second = second;
    node.as.triple.third = third;
    node.file = file;
    node.line = line;

    return add_node_to_ast(node);
}

#define node_unop(nk, arg) node_unop_loc(nk, arg, __LINE__, __FILE__)
#define node_binop(nk, lhs, rhs) node_binop_loc(nk, lhs, rhs, __LINE__, __FILE__)
#define node_triple(nk, first, second, third) node_triple_loc(nk, first, second, third, __LINE__, __FILE__)
#define node_number(n) node_number_loc(n, __LINE__, __FILE__)
#define node_x node_x_loc(__LINE__, __FILE__)
#define node_y node_y_loc(__LINE__, __FILE__)

/// @brief Print the AST
/// @param n 
void print_ast(Node* n){
    assert(n != NULL);

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

        case NK_E:
            printf("E(");
            print_ast(n->as.triple.first);
            printf(",");
            print_ast(n->as.triple.second);
            printf(",");
            print_ast(n->as.triple.third);
            printf(")");
            break;

        case NK_GTEQ:
            printf("geq(");
            print_ast(n->as.binop.lhs);
            printf(", ");
            print_ast(n->as.binop.rhs);
            printf(")");
            break;

        case NK_MOD:
            printf("mod(");
            print_ast(n->as.binop.lhs);
            printf(", ");
            print_ast(n->as.binop.rhs);
            printf(")");
            break;

        case NK_DIV:
            printf("div(");
            print_ast(n->as.binop.lhs);
            printf(", ");
            print_ast(n->as.binop.rhs);
            printf(")");
            break;

        case NK_SIN:
            printf("sin(");
            print_ast(n->as.unop);
            printf(")");
            break;

        case NK_COS:
            printf("cos(");
            print_ast(n->as.unop);
            printf(")");
            break;

        case NK_EXP:
            printf("exp(");
            print_ast(n->as.unop);
            printf(")");
            break;

        case NK_IF_THEN_ELSE:
            printf("if (");
            print_ast(n->as.triple.first);
            printf(") then { ");
            print_ast(n->as.triple.second);
            printf(" } else { ");
            print_ast(n->as.triple.third);
            printf("}");
            break;
        
        case NK_NUMBER:
            printf("%f", n->as.number); break;

        default:
    
            printf("[FILE %s] Node added at line %d ", n->file, n->line);
            printf("should not be able to reach this in print ast!\n");
            printf("\nkind %d\n", n->nk);

            exit(-1);
    }
}

#define print_ast_ln(node) (print_ast(node), printf("\n"))

void greyscale(){
    node_triple(NK_E, node_x, node_x, node_x);
}

void branch_func(){
    node_triple(NK_IF_THEN_ELSE,
        node_binop(NK_GTEQ, node_binop(NK_MULT, node_x, node_y), node_number(0)),
        node_triple(NK_E, node_x, node_y, node_number(1)),
        node_triple(NK_E,
            node_binop(NK_MOD,
                node_x, 
                node_y), 
            node_binop(NK_MOD,
                node_x, 
                node_y), 
            node_binop(NK_MOD,
                node_x,
                node_y)
        )
    );
}

void incorrect_ast(){
    node_triple(NK_E,
        node_x,
        node_x,
        node_triple(NK_E,
            node_x, 
            node_y, 
            node_x)
    );
}

#endif
