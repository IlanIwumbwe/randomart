#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdlib.h>
#include "ast.h"

typedef enum{
    BK_TRIPLE_RULE,
    BK_DOUBLE_RULE,
    BK_SINGLE_RULE,
    BK_SINGLE_RULE_NODE,
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
    char* name;
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

const size_t N_RULES = 3;
const size_t MAX_BRANCHES = 10;

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

void init_branches(Rule_kind rk, char* name, size_t capacity){

    if(g.used == N_RULES){
        printf("Define new rule and add memory for it\n");
        exit(-1);
    }

    Rule* rule = g.rule + rk;

    rule->branch = (Branch*) malloc(sizeof(Branch) * capacity);
    rule->kind = rk;
    rule->name = name;

    if(rule->branch == NULL){
        printf("[ERROR] Memory allocation of %ld elements failed!\n", capacity);
        exit(-1);
    }

    rule->capacity = capacity;
    rule->used = 0;

    g.used ++;
}

/// @brief Free memory used for branches by this rule
/// @param rule 
void free_branch_memory(Rule* rule){
    free(rule->branch);
    #ifdef DEBUG
    printf("Freed branch memory used by %s\n", rule->name);
    #endif
}

/// @brief For each rule, free memory used to store each branch, then free memory used to store the rule. This may free NULL pointer if you define a grammar that doesn't use all slots 
/// @brief providec by `N_RULES` 
void free_grammar(){
    for(size_t i = 0; i < g.capacity; ++i){
        free_branch_memory(g.rule + i);
    }

    free(g.rule);
    #ifdef DEBUG
    printf("Freed memory used by the grammar\n");
    #endif
}

void add_branch_to_rule(Rule_kind rk, Branch b){
    Rule* r = g.rule + rk;

    if(r->used >= r->capacity){
        r->capacity = 2 * r->capacity;

        Branch* nb = (Branch*)realloc(r->branch, sizeof(Branch) * r->capacity);
        
        if(nb == NULL){
            printf("[ERROR] Memory reallocation of branch failed!\n");
            exit(-1);
        }

        r->branch = nb; // move array pointer
    }

    r->branch[r->used++] = b;
    
    assert(r->used != 0);
}

Branch branch_single_rule_node(Rule_kind rk, Node_kind nk, float prob){
    assert(nk & NK_UNOP);

    Rule* rule = g.rule + rk;

    Branch b = {
        .kind = BK_SINGLE_RULE_NODE,
        .next_rule = {.one_rule = rule},
        .node_kind = nk,
        .prob = prob
    };

    return b;
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
    assert(nk & NK_BINOP);

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
    assert(nk & NK_TRIPLE);

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

Branch set_current_branch(Rule* rule, int depth){

    Branch current_branch = rule->branch[0];

    if((depth < 0) && (rule->kind != R_A)){   // A is a terminal rule, we have no need to force it to a particular branch
        return current_branch;
    }

    float branch_prob = randrange(0, 1); 
    float cummulative_prob = current_branch.prob;

    for(size_t i = 1; i < rule->used; ++i){
        if(branch_prob <= cummulative_prob){break;} // replace with cummulative prob here to choose nodes by paper probabilities
        
        current_branch = rule->branch[i];
        cummulative_prob += current_branch.prob;
    }

    return current_branch;
}

void print_branch(Branch* b){
    assert(b != NULL);

    if(b->kind == BK_SINGLE_RULE){  
        printf("%s", b->next_rule.one_rule->name);
    } else {
    
        switch(b->node_kind){
            case NK_X: 
                printf("x"); break;

            case NK_Y:
                printf("y"); break;

            case NK_ADD:
                printf("add(");
                printf("%s", b->next_rule.two_rules.lhs->name);
                printf(", ");
                printf("%s", b->next_rule.two_rules.rhs->name);
                printf(")");
                break;

            case NK_MULT:
                printf("mult(");
                printf("%s", b->next_rule.two_rules.lhs->name);
                printf(", ");
                printf("%s", b->next_rule.two_rules.rhs->name);
                printf(")");
                break;

            case NK_E:
                printf("(");
                printf("%s", b->next_rule.three_rules.first->name);
                printf(",");
                printf("%s", b->next_rule.three_rules.second->name);
                printf(",");
                printf("%s", b->next_rule.three_rules.third->name);
                printf(")");
                break;

            case NK_GEQ:
                printf("geq(");
                printf("%s", b->next_rule.two_rules.lhs->name);
                printf(", ");
                printf("%s", b->next_rule.two_rules.rhs->name);
                printf(")");
                break;

            case NK_MOD:
                printf("mod(");
                printf("%s", b->next_rule.two_rules.lhs->name);
                printf(", ");
                printf("%s", b->next_rule.two_rules.rhs->name);
                printf(")");
                break;

            case NK_DIV:
                printf("div(");
                printf("%s", b->next_rule.two_rules.lhs->name);
                printf(", ");
                printf("%s", b->next_rule.two_rules.rhs->name);
                printf(")");
                break;

            case NK_SIN:
                printf("sin(");
                printf("%s", b->next_rule.one_rule->name);
                printf(")");
                break;

            case NK_COS:
                printf("sin(");
                printf("%s", b->next_rule.one_rule->name);
                printf(")");
                break;

            case NK_EXP:
                printf("sin(");
                printf("%s", b->next_rule.one_rule->name);
                printf(")");
                break;

            case NK_IF_THEN_ELSE:
                printf("if(");
                printf("%s", b->next_rule.three_rules.first->name);
                printf(") then ");
                printf("%s", b->next_rule.three_rules.second->name);
                printf(" else ");
                printf("%s", b->next_rule.three_rules.third->name);
                break;
            
            case NK_NUMBER:
                printf("random number [-1 1]"); break;

            default:
                printf("Node added to grammar unknown!\n");
                exit(-1);
        }
    }

    printf(" [%f]", b->prob);
}

void print_branches(Rule rule){

    printf("%s ::= ", rule.name);
    
    for(size_t i = 0; i < rule.used; ++i){
        
        print_branch(rule.branch + i);        

        if(i != rule.used-1){
            printf(" | ");
        }
    }
}

void print_grammar(){
    printf("GRAMMAR: \n");
    for(size_t i = 0; i < g.capacity; ++i){
        if(g.rule[i].name){
            print_branches(g.rule[i]);
            printf("\n");
        }
    }

    printf("\n");
}

void grammar(){

    init_rules(N_RULES);

    init_branches(R_E, "E", MAX_BRANCHES); // allocate memory for branches
    init_branches(R_A, "A", MAX_BRANCHES); // allocate memory for branches
    init_branches(R_C, "C", MAX_BRANCHES); // allocate memory for branches

    g.entry_point = g.rule + R_E;

    assert(g.entry_point != NULL); // entry point must be defined

    add_branch_to_rule(R_E, branch_triple_rule(R_C, R_C, R_C, NK_E, 1));

    add_branch_to_rule(R_A, branch_no_rule(NK_NUMBER, 1.0/3.0));
    add_branch_to_rule(R_A, branch_no_rule(NK_X, 1.0/3.0));
    add_branch_to_rule(R_A, branch_no_rule(NK_Y, 1.0/3.0));

    add_branch_to_rule(R_C, branch_single_rule(R_A, 0.1));
    add_branch_to_rule(R_C, branch_double_rule(R_C, R_C, NK_ADD, 0.45));
    add_branch_to_rule(R_C, branch_single_rule_node(R_C, NK_COS, 0.45));
}

size_t generate_ast(Rule* rule, int depth){

    if(rule == NULL){
        printf("Rule is not defined!\n");
        exit(-1);
    }

    switch(rule->kind){
        case R_A: {
            Branch current_branch = set_current_branch(rule, depth);

            if(current_branch.node_kind == NK_NUMBER){
                return node_number(randrange(-1, 1));
            } else if (current_branch.node_kind == NK_X){
                return node_x;
            } else if(current_branch.node_kind == NK_Y) {
                return node_y;
            } else {
                printf("Rule A should only produce terminal nodes (number, x, y)!\n");
                exit(-1);
            }
        }

        case R_C: {

            Branch current_branch = set_current_branch(rule, depth);

            if(current_branch.kind == BK_SINGLE_RULE){
                return generate_ast(current_branch.next_rule.one_rule, depth - 1);

            } else if (current_branch.kind == BK_SINGLE_RULE_NODE){
                assert(current_branch.node_kind & NK_UNOP);
                size_t node = generate_ast(current_branch.next_rule.one_rule, depth - 1);

                return node_unop(current_branch.node_kind, node);

            } else if (current_branch.kind == BK_DOUBLE_RULE) {
                assert(current_branch.node_kind & NK_BINOP);

                size_t lhs = generate_ast(current_branch.next_rule.two_rules.lhs, depth - 1);
                size_t rhs = generate_ast(current_branch.next_rule.two_rules.rhs, depth - 1);

                return node_binop(current_branch.node_kind, lhs, rhs);

            } else {
                printf("Rule C should produce rule A, binary ops and unary ops!\n");
                exit(-1);
            }
        
        }

        case R_E: {

            Branch current_branch = set_current_branch(rule, depth);
                
            assert(current_branch.kind == BK_TRIPLE_RULE);
            
            size_t first = generate_ast(current_branch.next_rule.three_rules.first, depth - 1);
            size_t second = generate_ast(current_branch.next_rule.three_rules.second, depth - 1);
            size_t third = generate_ast(current_branch.next_rule.three_rules.third, depth - 1);

            return node_triple(current_branch.node_kind, first, second, third);
        }

        default:
            printf("This rule does not exist!\n");
            exit(-1);
    }


}

#endif

