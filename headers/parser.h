#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lex.h"
#include <errno.h>
#include <math.h>

int cursor;
int maybe_errors = 0;
int NUM_OF_TOKENS;

void consume(){
    if(cursor < NUM_OF_TOKENS - 1){
        cursor++;
    } else {
        printf("Cannot consume any more tokens! Cursor at %d / %d\n", cursor, NUM_OF_TOKENS);
        maybe_errors += 1;
    }
}

int token_matches(char* curr_token, char* expected){
    return !strcmp(curr_token, expected);
}

void expect_syntax(char* curr_token, char* expected){
    if(token_matches(curr_token, expected)){
        if(cursor == NUM_OF_TOKENS - 1){
            #ifdef DEBUG
            printf("%s must be the last token\n", curr_token);
            #endif
        } else {
            consume();
        }
        
    } else {
        printf("Expected %s but got %s \n", expected, curr_token);
        maybe_errors += 1;
    }
}

Option parse_C();

Option parse_binop(Node_kind nk){
    assert(nk & NK_BINOP);

    maybe_errors = 0;

    consume(); // consume binop func name

    Option lhs, rhs;

    expect_syntax(tokens[cursor], "(");
    lhs = parse_C();
    expect_syntax(tokens[cursor], ",");
    rhs = parse_C();
    expect_syntax(tokens[cursor], ")");

    return wrap_value(node_binop(nk, lhs.value, rhs.value), maybe_errors || lhs.none || rhs.none);
}

Option parse_unop(Node_kind nk){
    assert(nk & NK_UNOP);

    maybe_errors = 0;

    consume(); // consume unop func name

    Option node;

    expect_syntax(tokens[cursor], "(");
    node = parse_C();
    expect_syntax(tokens[cursor], ")");

    return wrap_value(node_unop(nk, node.value), maybe_errors || node.none);
}

Option parse_A(){
    maybe_errors = 0;

    if(token_matches(tokens[cursor], "x")){
        consume();

        return wrap_value(node_x, maybe_errors);

    } else if (token_matches(tokens[cursor], "y")){
        consume();

        return wrap_value(node_y, maybe_errors);

    } else {
        char* token = tokens[cursor];
        char* end;
        errno = 0;
        float num = strtof(token, &end);

        size_t node = 0;

        if(end == token){
            printf("Token %s is not a valid float!\n", token);
            maybe_errors = 1;

        } else if (errno == ERANGE){
            printf("Token %s is out of range!\n", token);
            maybe_errors = 1;

        } else if((num > 1.0) || (num < -1.0)){
            printf("Number must be in [-1, 1]!\n");
            maybe_errors = 1;

        } else {
            consume();
            node = node_number(num);
        }

        return wrap_value(node, maybe_errors);
    }
}

Option parse_C(){

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

    } else if (token_matches(tokens[cursor], "mult")){
        return parse_binop(NK_MULT);

    } else if (token_matches(tokens[cursor], "geq")){
        return parse_binop(NK_GEQ);

    } else {
        return parse_A();
    }

}

Option parse_E(){
    maybe_errors = 0;

    expect_syntax(tokens[cursor], "E");

    expect_syntax(tokens[cursor], "(");

    Option first = parse_C();
    expect_syntax(tokens[cursor], ",");
    Option second = parse_C();
    expect_syntax(tokens[cursor], ",");
    Option third = parse_C();

    expect_syntax(tokens[cursor], ")");

    return wrap_value(node_triple(NK_E, first.value, second.value, third.value), maybe_errors || first.none || second.none || third.none);
}

Option parse_if(){
    maybe_errors = 0;

    expect_syntax(tokens[cursor], "if");

    expect_syntax(tokens[cursor], "(");
    
    Option cond = parse_C();

    expect_syntax(tokens[cursor], ")");

    Option true_body = parse_E();

    expect_syntax(tokens[cursor], "else");

    Option false_body = parse_E();

    return wrap_value(node_triple(NK_IF_THEN_ELSE, cond.value, true_body.value, false_body.value), maybe_errors || cond.none || true_body.none || false_body.none);
}

int parse(char* input){
    NUM_OF_TOKENS = lex(input, AST_PATTERNS, sizeof(AST_PATTERNS) / sizeof(AST_PATTERNS[0]));
    cursor = 0;

    if(NUM_OF_TOKENS){
        Option ast_head;

        if(token_matches(tokens[cursor], "E")){
            ast_head = parse_E();

        } else if (token_matches(tokens[cursor], "if")){
            ast_head = parse_if();

        } else {
            printf("AST root should be if or E! Here %s is used \n", tokens[cursor]);
            return -1;
        }

        free_tokens(NUM_OF_TOKENS);

        return ast_head.none;

    } else {
        return -1;
    }
}


#endif