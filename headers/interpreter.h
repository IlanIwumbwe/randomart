#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "utils.h"

/// @brief Checks that the node evaluated correctly to a number node
/// @param n 
/// @return 
void expect_number(Node* n){

    if(n == NULL){
        printf("[FILE: %s] Node added at line %d failed to evaluate!\n", n->file, n->line);
        exit(-1);
    }
    
    if(n->nk != NK_NUMBER){
        printf("[FILE: %s] Node added at line %d cannot evaluate to a number!\n", n->file, n->line);
        exit(-1);
    }
}

/// @brief Interpret the AST
/// @param n 
/// @param x 
/// @param y 
/// @return 
size_t eval_ast(size_t index, float x, float y){
    Node* n = ast.array + index;

    switch(n->nk){
        case NK_X: 
            return node_number_loc(x, n->line, n->file);
            
        case NK_Y:
            return node_number_loc(y, n->line, n->file);

        case NK_ADD:{
            Node* lhs_eval = ast.array + eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = ast.array + eval_ast(n->as.binop.rhs, x, y);

            expect_number(lhs_eval);
            expect_number(rhs_eval);

            return node_number_loc(lhs_eval->as.number + rhs_eval->as.number, n->line, n->file);
        }

        case NK_MULT: {
            Node* lhs_eval = ast.array + eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = ast.array + eval_ast(n->as.binop.rhs, x, y);

            expect_number(lhs_eval);
            expect_number(rhs_eval);

            return node_number_loc(lhs_eval->as.number * rhs_eval->as.number, n->line, n->file);
        }

        case NK_E: {

            size_t first_eval_index = eval_ast(n->as.triple.first, x, y);
            size_t second_eval_index = eval_ast(n->as.triple.second, x, y);
            size_t third_eval_index = eval_ast(n->as.triple.third, x, y);

            expect_number(ast.array + first_eval_index);
            expect_number(ast.array + second_eval_index);
            expect_number(ast.array + third_eval_index);

            return node_triple_loc(
                NK_E,
                first_eval_index,
                second_eval_index,
                third_eval_index,
                n->line,
                n->file
            );
        }

        case NK_GEQ: {
            Node* lhs_eval = ast.array + eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = ast.array + eval_ast(n->as.binop.rhs, x, y);

            expect_number(lhs_eval);
            expect_number(rhs_eval);

            return node_number_loc(lhs_eval->as.number >= rhs_eval->as.number, n->line, n->file);
        }

        case NK_MOD: {
            Node* lhs_eval = ast.array + eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = ast.array + eval_ast(n->as.binop.rhs, x, y);

            expect_number(lhs_eval);
            expect_number(rhs_eval);

            if(rhs_eval->as.number == 0.0){
                rhs_eval->as.number = 1.0;
            }

            return node_number_loc(fmod(lhs_eval->as.number, rhs_eval->as.number), n->line, n->file);
        }

        case NK_DIV: {

            Node* lhs_eval = ast.array + eval_ast(n->as.binop.lhs, x, y);
            Node* rhs_eval = ast.array + eval_ast(n->as.binop.rhs, x, y);

            expect_number(lhs_eval);
            expect_number(rhs_eval);

            if(rhs_eval->as.number == 0.0){
                rhs_eval->as.number = 1.0;
            }

            return node_number_loc(lhs_eval->as.number / rhs_eval->as.number, n->line, n->file);
        }
        
        case NK_SIN: {

            Node* eval = ast.array + eval_ast(n->as.unop, x, y);

            expect_number(eval);
            
            return node_number_loc(sin(eval->as.number), n->line, n->file);
        }

        case NK_COS: {

            Node* eval = ast.array + eval_ast(n->as.unop, x, y);

            expect_number(eval);
            
            return node_number_loc(cos(eval->as.number), n->line, n->file);
        }        

        case NK_EXP: {
            Node* eval = ast.array + eval_ast(n->as.unop, x, y);

            expect_number(eval);
            
            return node_number_loc(exp(eval->as.number), n->line, n->file);
        }

        case NK_IF_THEN_ELSE: {

            size_t cond_eval_index = eval_ast(n->as.triple.first, x, y);
            Node* cond_eval = ast.array + cond_eval_index;

            expect_number(cond_eval);
       
            if(cond_eval->as.number){
                return eval_ast(n->as.triple.second, x, y);
            } else {
                return eval_ast(n->as.triple.third, x, y);
            }
        }

        case NK_NUMBER:
            return index;

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
size_t eval(float x, float y){
    find_ast_root();
    
    return eval_ast(ast.array_head, x, y);
}

/// @brief Sample AST at random points
void test_eval(){
    size_t res = eval(randrange(-1, 1), randrange(-1,1));

    printf("Result of evaluation: \n");
    print_ast_ln(res);
    printf("Nodes added during evaluation: %ld\n", ast.used - ast.size);
}

#endif

