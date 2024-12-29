#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "utils.h"

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

        case NK_TRIPLE: {

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
                node_number_loc(first_eval->as.number, n->as.triple.first->line, n->as.triple.first->file), 
                node_number_loc(second_eval->as.number, n->as.triple.second->line, n->as.triple.second->file), 
                node_number_loc(third_eval->as.number, n->as.triple.third->line, n->as.triple.third->file),
                n->line,
                n->file
            );
        }

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
    find_ast_root(ast);
    
    Node* res = eval_ast(ast->array_head, x, y);

    #ifdef DEBUG
    printf("Nodes added during evaluation: %ld\n", ast->used - ast->size);
    #endif

    return res;
}

#endif

