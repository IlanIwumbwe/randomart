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

#define NK_UNOP (NK_SIN, NK_COS, NK_EXP)
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

        if(n.nk & NK_BINOP){
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

Node* node_binop_loc(Node_kind nk, Node* lhs, Node* rhs, int line, char* file){
    assert(nk & NK_BINOP);

    Node node;
    node.nk = nk;

    node.as.binop.lhs = lhs;
    node.as.binop.rhs = rhs;
    node.file = file;
    node.line = line;

    return add_node_to_ast(&ast, node);
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

    return add_node_to_ast(&ast, node);
}

#define node_binop(nk, lhs, rhs) node_binop_loc(nk, lhs, rhs, __LINE__, __FILE__)
#define node_triple(nk, first, second, third) node_triple_loc(nk, first, second, third, __LINE__, __FILE__)
#define node_number(n) node_number_loc(n, __LINE__, __FILE__)
#define node_x node_x_loc(__LINE__, __FILE__)
#define node_y node_y_loc(__LINE__, __FILE__)

Node* node_C(int depth);

Node* node_E(int depth){
    float branch_prob = randrange(0, 1);

    if((branch_prob < 0.5) || (depth == 0)){
        return node_triple(NK_E, node_C(depth), node_C(depth), node_C(depth));
    } else {
        return node_triple(NK_IF_THEN_ELSE, node_C(depth), node_E(depth - 1), node_E(depth - 1));
    }
}

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
        return node_binop(NK_ADD, node_C(depth - 1), node_C(depth - 1));

    } else {
        return node_binop(NK_MULT, node_C(depth - 1), node_C(depth - 1));
    }

    // C

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
            print_ast(n->as.binop.lhs);
            printf(" >= ");
            print_ast(n->as.binop.rhs);
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
            printf("Should not be able to reach this in print ast!\n");
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

void build_ast(int depth){
    init_ast(&ast, 20);

    // build AST. E, A and C nodes will follow the grammar rules to generate the AST
    node_E(depth);
    
    ast.size = ast.used; // set size of AST right after generating it

    print_ast_ln(ast.array_head);

    printf("nodes in AST: %ld\n", ast.size);

    /*
        In the worst case scenario, we need to create as many nodes as there are in the AST in order to evaluate it. This happens if there's no number nodes
        in the AST. This means that if we want no reallocations to happen during evaluation i.e we never run our of space in the dynamic array, we need to make 
        sure that the AST is stored with a capacity that's at least twice as big as the AST size. 

        5 added just to be extra extra safe
    */

    reallocate_ast(&ast, 2 * (ast.size + 5));
}

#endif
