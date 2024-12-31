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
    assert(n != NULL);

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

        case NK_E: {

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
                NK_E,
                first_eval,
                second_eval,
                third_eval,
                n->line,
                n->file
            );
        }

        case NK_GTEQ: {
            Node* lhs_eval = eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = eval_ast(n->as.binop.rhs, x, y);

            if(!expect_number(lhs_eval)){
                return NULL;
            }

            if(!expect_number(rhs_eval)){
                return NULL;
            }

            return node_number_loc(lhs_eval->as.number >= rhs_eval->as.number, n->line, n->file);
        }

        case NK_MOD: {
            Node* lhs_eval = eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = eval_ast(n->as.binop.rhs, x, y);

            if(!expect_number(lhs_eval)){
                return NULL;
            }

            if(!expect_number(rhs_eval)){
                return NULL;
            }

            if(rhs_eval->as.number == 0.0){
                rhs_eval->as.number = 1.0;
            }

            return node_number_loc(fmod(lhs_eval->as.number, rhs_eval->as.number), n->line, n->file);
        }

        case NK_DIV: {

            Node* lhs_eval = eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = eval_ast(n->as.binop.rhs, x, y);

            if(!expect_number(lhs_eval)){
                return NULL;
            }

            if(!expect_number(rhs_eval)){
                return NULL;
            }

            if(rhs_eval->as.number == 0.0){
                rhs_eval->as.number = 1.0;
            }

            return node_number_loc(lhs_eval->as.number / rhs_eval->as.number, n->line, n->file);
        }
        
        case NK_SIN: {

            Node* eval = eval_ast(n->as.unop, x, y);

            if(!expect_number(eval)){
                return NULL;
            }
            
            return node_number_loc(sin(eval->as.number), n->line, n->file);
        }

        case NK_COS: {

            Node* eval = eval_ast(n->as.unop, x, y);

            if(!expect_number(eval)){
                return NULL;
            }
            
            return node_number_loc(cos(eval->as.number), n->line, n->file);
        }        

        case NK_EXP: {
            Node* eval = eval_ast(n->as.unop, x, y);

            if(!expect_number(eval)){
                return NULL;
            }
            
            return node_number_loc(exp(eval->as.number), n->line, n->file);
        }

        case NK_IF_THEN_ELSE: {

            Node* cond_eval = eval_ast(n->as.triple.first, x, y);
       
            if(!expect_number(cond_eval)){
                return NULL;
            }

            if(cond_eval->as.number){
                return eval_ast(n->as.triple.second, x, y);
            } else {
                return eval_ast(n->as.triple.third, x, y);
            }
        }

        case NK_NUMBER:
            return n;

        default:
            if(n == NULL){
                printf("Node is NULL!\n");
            } else {
                printf("[FILE %s] Node added at line %d ", n->file, n->line);
                printf("should not be able to reach this in eval ast!\n");
                printf("\nkind %d\n", n->nk);
            }
            
            exit(-1);
    }
}

/// @brief Evaluate given AST. After evaluation, reset head to point to AST state before evaluation
/// @param ast 
/// @param x 
/// @param y 
/// @return `Node*` which holds the result
Node* eval(float x, float y){
    find_ast_root();
    
    return eval_ast(ast.array_head, x, y);
}

/// @brief Sample AST at random points
void test_eval(){
    Node* res = eval(randrange(-1, 1), randrange(-1,1));

    if(res == NULL){
        printf("[%s] AST is invalid! Cannot evaluate it\n", __FILE__);
    } else {
        printf("Result of evaluation: \n");
        print_ast_ln(res);
        printf("Nodes added during evaluation: %ld\n", ast.used - ast.size);
    }
}

#endif

