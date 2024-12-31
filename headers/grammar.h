#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdlib.h>
#include "ast.h"

typedef enum{
    BK_TRIPLE_RULE,
    BK_DOUBLE_RULE,
    BK_SINGLE_RULE,
    BK_NO_RULE
} Branch_kind;

typedef enum {
    R_E, R_A, R_C
} Rule_kind;

typedef struct sRule Rule;

typedef struct {
    Rule* lhs;
    Rule* rhs;
} NT_Binop;

typedef struct {
    Rule* first;
    Rule* second;
    Rule* third;
} NT_Triple;

typedef union {
    Rule* one_rule;
    NT_Binop two_rules;
    NT_Triple three_rules;
} Next_rule;

typedef struct {
    Branch_kind kind;
    Next_rule next_rule;
    Node_kind node_kind;

    float prob;
} Branch;

struct sRule {
    Rule_kind kind;
    Branch* branch;

    size_t used;
    size_t capacity;
};

typedef struct {
    Rule* rule;

    Rule* entry_point;
    size_t used;
    size_t capacity;
} Grammar;

Grammar g = {0};

const int N_RULES = 3;
const int MAX_BRANCHES = 3;

/// @brief Allocate memory for all rules that should be added to the grammar
/// @param g 
void init_rules(size_t capacity){
    g.rule = (Rule*) malloc(sizeof(Rule) * capacity);

    if(g.rule == NULL){
        printf("[ERROR] Memory allocation of %ld elements failed!\n", capacity);
        exit(-1);
    }

    g.capacity = capacity;
    g.used = 0;
}

void init_branches(Rule_kind rk, size_t capacity){
    Rule* rule = g.rule + rk;

    rule->branch = (Branch*) malloc(sizeof(Branch) * capacity);
    rule->kind = rk;

    if(rule->branch == NULL){
        printf("[ERROR] Memory allocation of %ld elements failed!\n", capacity);
        exit(-1);
    }

    rule->capacity = capacity;
    rule->used = 0;
}

/// @brief Free memory used for branches by this rule
/// @param rule 
void free_branch_memory(Rule* rule){
    free(rule->branch);
    printf("Freed branch memory used by this rule\n");
}

/// @brief For each rule, free memory used to store each branch, then free memory used to store the rule
void free_grammar(){
    for(size_t i = 0; i < g.used; ++i){
        free_branch_memory(g.rule + i);
        free(g.rule);
    }
    printf("Freed memory used by the grammar\n");
}

void add_branch_to_rule(Rule_kind rk, Branch b){
    Rule* r = g.rule + rk;

    if(r->used >= r->capacity){
        r->capacity = 2 * r->capacity;

        Branch* nb = (Branch*)realloc(r->branch, sizeof(Branch) * r->capacity);
        
        if(nb == NULL){
            printf("[ERROR] Memory reallocation of failed!\n");
            exit(-1);
        }

        r->branch = nb; // move array pointer
    }

    // if this isn't the first branch to be added for this rule, make the probability cummulative
    // this makes picking a branch easier and cleaner to write
    if(r->used != 0){
        b.prob += r->branch[r->used-1].prob;
    }

    r->branch[r->used++] = b;
    
    assert(r->used != 0);
}

Branch branch_single_rule(Rule_kind rk, float prob){
    Rule* rule = g.rule + rk;

    Branch b = {
        .kind = BK_SINGLE_RULE,
        .next_rule = {.one_rule = rule},
        .prob = prob
    };

    return b;
}

Branch branch_double_rule(Rule_kind lhs_rk, Rule_kind rhs_rk, Node_kind nk, float prob){
    Rule* lhs = g.rule + lhs_rk;
    Rule* rhs = g.rule + rhs_rk;

    Branch b = {
        .kind = BK_DOUBLE_RULE,
        .next_rule = {.two_rules = {lhs, rhs}},
        .node_kind = nk,
        .prob = prob
    };

    return b;
}

Branch branch_triple_rule(Rule_kind first_rk, Rule_kind second_rk, Rule_kind third_rk, Node_kind nk, float prob){
    Rule* first = g.rule + first_rk;
    Rule* second = g.rule + second_rk;
    Rule* third = g.rule + third_rk;

    Branch b = {
        .kind = BK_TRIPLE_RULE,
        .next_rule = {.three_rules = {first, second, third}},
        .node_kind = nk,
        .prob = prob
    };

    return b;
}

Branch branch_no_rule(Node_kind nk, float prob){
    Branch b = {
        .kind = BK_NO_RULE,
        .node_kind = nk,
        .prob = prob
    };

    return b;
}

void paper_grammar(){

    init_rules(N_RULES);

    init_branches(R_E, MAX_BRANCHES); // allocate memory for branches
    init_branches(R_A, MAX_BRANCHES); // allocate memory for branches
    init_branches(R_C, MAX_BRANCHES); // allocate memory for branches

    g.entry_point = g.rule + R_E;

    assert(g.entry_point != NULL); // entry point must be defined

    add_branch_to_rule(R_E, branch_triple_rule(R_C, R_C, R_C, NK_E, 1.0));

    add_branch_to_rule(R_A, branch_no_rule(NK_NUMBER, 1.0/3.0));
    add_branch_to_rule(R_A, branch_no_rule(NK_X, 1.0/3.0));
    add_branch_to_rule(R_A, branch_no_rule(NK_Y, 1.0/3.0));

    add_branch_to_rule(R_C, branch_single_rule(R_A, 1.0/3.0));
    add_branch_to_rule(R_C, branch_double_rule(R_C, R_C, NK_ADD, 3.0/8.0));
    add_branch_to_rule(R_C, branch_double_rule(R_C, R_C, NK_MULT, 3.0/8.0));
}

void simple_grammar(){

    init_rules(N_RULES);

    init_branches(R_A, MAX_BRANCHES); // allocate memory for branches

    g.entry_point = g.rule + R_A;

    assert(g.entry_point != NULL); // entry point must be defined

    add_branch_to_rule(R_A, branch_no_rule(NK_NUMBER, 1.0/3.0));
    add_branch_to_rule(R_A, branch_no_rule(NK_X, 1.0/3.0));
    add_branch_to_rule(R_A, branch_no_rule(NK_Y, 1.0/3.0));
}

Node* generate_ast(Rule* rule, int depth){

    if(rule == NULL){
        printf("Rule is not defined!\n");
        exit(-1);
    }

    switch(rule->kind){
        case R_A: {
            float branch_prob = randrange(0, 1);
            Branch current_branch = rule->branch[0];

            for(size_t i = 0; i < rule->used; ++i){
                current_branch = rule->branch[i];
                
                if(branch_prob <= current_branch.prob){break;}
            }

            if(current_branch.node_kind == NK_NUMBER){
                return node_number(randrange(-1, 1));
            } else if (current_branch.node_kind == NK_X){
                return node_x;
            } else {
                return node_y;
            }
        }

        case R_C: {

            if(depth == 0){
                return generate_ast(g.rule + R_A, 0);
            }

            float branch_prob = randrange(0, 1);
            Branch current_branch = rule->branch[0];

            for(size_t i = 0; i < rule->used; ++i){
                current_branch = rule->branch[i];
                
                if(branch_prob <= current_branch.prob){break;}
            }

            if(current_branch.kind == BK_SINGLE_RULE){
                return generate_ast(current_branch.next_rule.one_rule, depth - 1);

            } else if (current_branch.kind == BK_DOUBLE_RULE) {
                assert(current_branch.node_kind & NK_BINOP);

                Node* lhs = generate_ast(current_branch.next_rule.two_rules.lhs, depth - 1);
                Node* rhs = generate_ast(current_branch.next_rule.two_rules.rhs, depth - 1);

                return node_binop(current_branch.node_kind, lhs, rhs);

            } else {
                printf("Rule C should only produce rule A and binary ops!\n");
                exit(-1);
            }
        
        }

        case R_E: {
        
            assert(rule->used == 1);
            
            Branch current_branch = rule->branch[0];

            assert(current_branch.kind == BK_TRIPLE_RULE);
            
            Node* first = generate_ast(current_branch.next_rule.three_rules.first, depth - 1);
            Node* second = generate_ast(current_branch.next_rule.three_rules.second, depth - 1);
            Node* third = generate_ast(current_branch.next_rule.three_rules.third, depth - 1);

            return node_triple(current_branch.node_kind, first, second, third);
        }

        default:
            printf("This rule does not exist!\n");
            exit(-1);
    }

}

#endif

