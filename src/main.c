#include <stdio.h>
#include <time.h>
#include "../headers/run.h"

int main(){

    run();

    free_ast();
    free_grammar();
    
    return 0;
}

