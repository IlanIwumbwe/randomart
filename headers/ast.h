#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

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
};

typedef struct{
    Node* n;
    size_t used;
    size_t capacity;

    Node* root;
    size_t size; // size of AST after initial generation
} Ast;

extern Ast ast;

void init_ast(Ast* ast, size_t capacity){

    if(capacity <= 0){
        printf("[ERROR] Cannot initialise dynamic array with capacity of %ld!\n", capacity);
        exit(-1);
    } else {
        ast->n = (Node*) malloc(sizeof(Node) * capacity);
        
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

void reallocate_node_pointers(Node n, Node* old_node_loc, Node* new_node_loc){

    if(old_node_loc != new_node_loc){

        if((n.nk == NK_ADD) || (n.nk == NK_MULT)){
            unsigned long lhs_offset = old_node_loc - n.as.binop.lhs;
            unsigned long rhs_offset = old_node_loc - n.as.binop.rhs;

            assert(lhs_offset > 0);
            assert(rhs_offset > 0);

            new_node_loc->as.binop.lhs = new_node_loc - lhs_offset;
            new_node_loc->as.binop.rhs = new_node_loc - rhs_offset;
        }

        if(n.nk == NK_TRIPLE){
            unsigned long first_offset = old_node_loc - n.as.triple.first;
            unsigned long second_offset = old_node_loc - n.as.triple.second;
            unsigned long third_offset = old_node_loc - n.as.triple.third;

            assert(first_offset > 0);
            assert(second_offset > 0);
            assert(third_offset > 0);

            printf("reallocating for node in file %s at line %d\n", n.file, n.line);

            new_node_loc->as.triple.first = new_node_loc - first_offset;
            new_node_loc->as.triple.second = new_node_loc - second_offset;
            new_node_loc->as.triple.third = new_node_loc - third_offset;
        }

    }
}

Node* add_node_to_ast(Ast* ast, Node node){
    Node* old_ast_loc = ast->n;

    if(ast->used >= ast->capacity){
        ast->capacity = 2 * ast->capacity;
        
        Node* nn = (Node*)realloc(ast->n, sizeof(Node) * ast->capacity);

        if(nn == NULL){
            printf("[ERROR] Memory reallocation of failed!\n");
            free_ast(ast);
            exit(-1);
        }

        // move pointers of nodes to point to the new memory locations    
        for (size_t i = 0; i < ast->size; ++i){
            reallocate_node_pointers(ast->n[i], ast->n+i, nn+i);
        }

        ast->n = nn; // move array pointer
    }

    ast->n[ast->used++] = node;

    reallocate_node_pointers(node, old_ast_loc + ast->used, ast->n + ast->used); // useful if a reallocation happens while building the AST  

    assert(ast->used != 0);

    ast->root = ast->n + ast->used - 1;

    return ast->root;
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

/// @brief Print the AST
/// @param n 
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

/// @brief Checks that the node evaluated correctly to a number node
/// @param n 
/// @return 
Node* expect_number(Node* n){

    if(n == NULL){
        printf("[FILE: %s] Node added at line %d failed to evaluate!\n", n->file, n->line);
        return NULL;
    }
    
    if(n->nk != NK_NUMBER){
        printf("[FILE: %s] Node added at line %d cannot evaluate to a number!\n", n->file, n->line);
        return NULL;
    }

    return n;
}

/// @brief Interpret the AST
/// @param n 
/// @param x 
/// @param y 
/// @return 
Node* eval_ast(Node* n, float x, float y){

    switch(n->nk){
        case NK_X: 
            return node_number_loc(x, n->line, n->file);
            
        case NK_Y:
            return node_number_loc(y, n->line, n->file);

        case NK_ADD:{
            Node* lhs_eval = eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = eval_ast(n->as.binop.rhs, x, y);

            if(!expect_number(lhs_eval)){
                return NULL;
            }

            if(!expect_number(rhs_eval)){
                return NULL;
            }

            return node_number_loc(lhs_eval->as.number + rhs_eval->as.number, n->line, n->file);
        }

        case NK_MULT: {
            Node* lhs_eval = eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = eval_ast(n->as.binop.rhs, x, y);

            if(!expect_number(lhs_eval)){
                return NULL;
            }

            if(!expect_number(rhs_eval)){
                return NULL;
            }

            return node_number_loc(lhs_eval->as.number * rhs_eval->as.number, n->line, n->file);
        }

        case NK_TRIPLE:

            Node* first_eval = eval_ast(n->as.triple.first, x, y);
            Node* second_eval = eval_ast(n->as.triple.second, x, y);
            Node* third_eval = eval_ast(n->as.triple.third, x, y);

            if(!expect_number(first_eval)){
                return NULL;
            }

            if(!expect_number(second_eval)){
                return NULL;
            }

            if(!expect_number(third_eval)){
                return NULL;
            }

            return node_triple_loc(
                node_number_loc(first_eval->as.number, first_eval->line, first_eval->file), 
                node_number_loc(second_eval->as.number, second_eval->line, second_eval->file), 
                node_number_loc(third_eval->as.number, third_eval->line, third_eval->file),
                n->line,
                n->file
            );

        case NK_NUMBER:
            return n;

        default:
            printf("[FILE %s] Node added at line %d ", n->file, n->line);
            printf("should not be able to reach this in eval ast!\n");

            exit(-1);
    }
}

/// @brief Evaluate given AST. After evaluation, reset head to point to AST state before evaluation
/// @param ast 
/// @param x 
/// @param y 
/// @return `Node*` which holds the result
Node* eval(Ast* ast, float x, float y){
    ast->root = ast->n + ast->size - 1; // reset head pointer to top of AST to setup re-evaluation
    ast->used = ast->size; // reset used counter to overwrite created nodes during previous evaluation
    
    Node* res = eval_ast(ast->root, x, y);

    return res;
}

#define print_ast_ln(node) (print_ast(node), printf("\n"))

#endif

