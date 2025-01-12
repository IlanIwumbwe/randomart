#ifndef LEX_H
#define LEX_H

#include <regex.h>
#include <stdio.h>

#include "utils.h"

char* tokens[INPUT_SIZE] = {0};

size_t lex(char* input, const char* PATTERNS[], size_t num_of_patterns){
    size_t curr_token = 0;

    regex_t regex[num_of_patterns];
    regmatch_t match;

    for(size_t i = 0; i < num_of_patterns; ++i){
        if(regcomp(&regex[i], PATTERNS[i], REG_EXTENDED) != 0){
            printf("Could not compile regex pattern!\n");
            exit(-1);
        }
    }

    char* cursor = input;

    while(*cursor != '\0'){
        int matched = 0;

        if((*cursor == '\n') || (*cursor == '\t') || (*cursor == ' ')){
            cursor ++;
            continue;
        }

        for(size_t i = 0; i < num_of_patterns; ++i){

            if(!regexec(&regex[i], cursor, 1, &match, 0) && !match.rm_so){
                size_t length = match.rm_eo - match.rm_so;

                tokens[curr_token] = (char*) malloc(length + 1);
                memcpy(tokens[curr_token], cursor, length);
                tokens[curr_token][length] = '\0';

                curr_token++;
                
                cursor += length;
                matched = 1;
                break;
            }

        }

        if(!matched){
            printf("Could not match any of the known patterns at char %c \n", *cursor);
            curr_token = 0;
            break;
        }

    }

    for(size_t i = 0; i < num_of_patterns; ++i){
        regfree(&regex[i]);
    }

    #ifdef DEBUG
    for(size_t i = 0; i < curr_token; ++i){
        printf("val: %s \n", tokens[i]);
    }
    #endif

    return curr_token;

}

void free_tokens(size_t num_of_tokens){
    for(size_t i = 0; i < num_of_tokens; ++i){
        free(tokens[i]);
    }
    #ifdef DEBUG
    printf("Freed memory used to store tokens\n");
    #endif
}

#endif


