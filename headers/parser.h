#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lex.h"
#include <errno.h>
#include <math.h>

int cursor;
int NUM_OF_TOKENS;

void consume(){
    if((cursor < NUM_OF_TOKENS - 1) || (cursor == NUM_OF_TOKENS - 1)){
        cursor++;
    } else {
        printf("Cannot consume any more tokens! Cursor at %d / %d\n", cursor, NUM_OF_TOKENS);
        exit(-1);
    }
}

int token_matches(char* curr_token, char* expected){
    return !strcmp(curr_token, expected);
}

void expect_syntax(char* curr_token, char* expected){
    if(token_matches(curr_token, expected)){
        consume();
    } else {
        printf("Expected %s but got %s \n", expected, curr_token);
        exit(-1);
    }
}

size_t parse_C();

size_t parse_binop(Node_kind nk){
    assert(nk & NK_BINOP);
    consume(); // consume binop func name

    size_t lhs = 0, rhs = 0;

    expect_syntax(tokens[cursor], "(");
    lhs = parse_C();
    expect_syntax(tokens[cursor], ",");
    rhs = parse_C();
    expect_syntax(tokens[cursor], ")");

    return node_binop(nk, lhs, rhs);
}

size_t parse_unop(Node_kind nk){
    assert(nk & NK_UNOP);
    consume(); // consume unop func name

    size_t node = 0;

    expect_syntax(tokens[cursor], "(");
    node = parse_C();
    expect_syntax(tokens[cursor], ")");

    return node_unop(nk, node);
}

size_t parse_A(){

    if(token_matches(tokens[cursor], "x")){
        consume();
        return node_x;
    } else if (token_matches(tokens[cursor], "y")){
        consume();
        return node_y;
    } else {
        char* token = tokens[cursor];
        char* end;
        errno = 0;
        float num = strtof(token, &end);

        if(end == token){
            printf("Token %s is not a valid float!\n", token);
            exit(-1);
        } else if (errno == ERANGE){
            printf("Token %s is out of range!\n", token);
            exit(-1);
        } else if((num > 1.0) || (num < -1.0)){
            printf("Number must be in [-1, 1]!\n");
            exit(-1);
        } else {
            consume();
            return node_number(num);
        }
    }
}

size_t parse_C(){
    
    if(token_matches(tokens[cursor], "add")){
        return parse_binop(NK_ADD);
        
    } else if (token_matches(tokens[cursor], "div")){
        return parse_binop(NK_DIV);

    } else if (token_matches(tokens[cursor], "sin")){
        return parse_unop(NK_SIN);

    } else if (token_matches(tokens[cursor], "cos")){
        return parse_unop(NK_COS);

    } else if (token_matches(tokens[cursor], "exp")){
        return parse_unop(NK_EXP);

    } else if (token_matches(tokens[cursor], "mod")){
        return parse_binop(NK_MOD);

    } else {
        return parse_A();
    }

}

size_t parse_E(){
    expect_syntax(tokens[cursor], "E");

    expect_syntax(tokens[cursor], "(");

    size_t first = parse_C();
    expect_syntax(tokens[cursor], ",");
    size_t second = parse_C();
    expect_syntax(tokens[cursor], ",");
    size_t third = parse_C();

    expect_syntax(tokens[cursor], ")");

    return node_triple(NK_E, first, second, third);
}

size_t parse_if(){
    expect_syntax(tokens[cursor], "if");

    expect_syntax(tokens[cursor], "(");
    
    size_t cond = parse_C();

    expect_syntax(tokens[cursor], ")");

    size_t true_body = parse_E();

    expect_syntax(tokens[cursor], "else");

    size_t false_body = parse_E();

    return node_triple(NK_IF_THEN_ELSE, cond, true_body, false_body);
}

int parse(char* input){
    NUM_OF_TOKENS = lex(input);
    cursor = 0;

    if(NUM_OF_TOKENS){
        if(token_matches(tokens[cursor], "E")){
            parse_E();

        } else if (token_matches(tokens[cursor], "if")){
            parse_if();
        }

        free_tokens(NUM_OF_TOKENS);
        return 0;
    } else {
        return -1;
    }
}


#endif