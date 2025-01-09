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
    RK_NORMAL = set_bit(0),
    RK_TERMINAL = set_bit(1),
    RK_ENTRY = set_bit(2),
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
    char* name;
    Rule_kind rk;

    Branch* branch;

    size_t used;
    size_t capacity;
};

typedef struct {
    Rule* rule;
    size_t used;
    size_t capacity;

    Rule* entry_point;
    Rule* terminal_rule;
} Grammar;

Grammar g = {0};

const size_t N_RULES = 3;
const size_t MAX_BRANCHES = 10;

/// @brief Allocate memory for all rules that should be added to the grammar
/// @param g 
void init_grammar(size_t capacity){
    g.rule = (Rule*) malloc(sizeof(Rule) * capacity);

    if(g.rule == NULL){
        printf("[ERROR] Memory allocation of %ld elements failed!\n", capacity);
        exit(-1);
    }

    g.capacity = capacity;
    g.used = 0;
}

/// @brief Init memory used by branches of each rule
/// @param rule 
/// @param capacity 
void init_branches(size_t capacity){
    Rule* rule;

    for(size_t i = 0; i < g.used; ++i){
        rule = g.rule + i;

        rule->branch = (Branch*) malloc(sizeof(Branch) * capacity);

        if(rule->branch == NULL){
            printf("[ERROR] Memory allocation of %ld elements failed!\n", capacity);
            exit(-1);
        }

        rule->capacity = capacity;
        rule->used = 0;
    }
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

/// @brief Allocate memory for rule with name `rule_name` into the gramar
/// @param rule_name 
/// @param num_of_branches 
void add_rule_to_grammar(char* rule_name, Rule_kind rk_flag){

    if(g.used == g.capacity){
        g.capacity = 2 * g.capacity;

        Rule* nr = (Rule*)realloc(g.rule, sizeof(Rule) * g.capacity);
        
        if(nr == NULL){
            printf("[ERROR] Memory reallocation of branch failed!\n");
            exit(-1);
        }

        g.rule = nr; 
    }

    g.rule[g.used++] = (Rule){.name = rule_name, .rk = rk_flag};

    if(rk_flag & RK_ENTRY){
        g.entry_point = g.rule + g.used - 1;
    }

    if(rk_flag & RK_TERMINAL){
        g.terminal_rule = g.rule + g.used - 1;
    }
}

Rule* find_rule_location(char* rule_name){
    char* name;

    for(size_t i = 0; i < g.used; ++i){
        name = g.rule[i].name;

        if(!strcmp(name, rule_name)){
            return g.rule + i;
        }
    }

    return NULL;
}

Rule* expect_rule(char* rule_name){
    Rule* r = find_rule_location(rule_name);

    if(r == NULL){
        printf("Rule %s was not added to the grammar! Define it first \n", rule_name);
        exit(-1);
    }

    return r;
}

void add_branch_to_rule(char* rule_name, Branch b){
    Rule* r = expect_rule(rule_name);

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

Branch branch_single_rule_node(char* rule_name, Node_kind nk, float prob){
    assert(nk & NK_UNOP);

    Rule* rule = expect_rule(rule_name);

    Branch b = {
        .kind = BK_SINGLE_RULE_NODE,
        .next_rule = {.one_rule = rule},
        .node_kind = nk,
        .prob = prob
    };

    return b;
}

Branch branch_single_rule(char* rule_name, float prob){
    Rule* rule = expect_rule(rule_name);

    Branch b = {
        .kind = BK_SINGLE_RULE,
        .next_rule = {.one_rule = rule},
        .prob = prob
    };

    return b;
}

Branch branch_double_rule(char* lhs_rule_name, char* rhs_rule_name, Node_kind nk, float prob){
    assert(nk & NK_BINOP);

    Rule* lhs = expect_rule(lhs_rule_name);
    Rule* rhs = expect_rule(rhs_rule_name);

    Branch b = {
        .kind = BK_DOUBLE_RULE,
        .next_rule = {.two_rules = {lhs, rhs}},
        .node_kind = nk,
        .prob = prob
    };

    return b;
}

Branch branch_triple_rule(char* first_rule_name, char* second_rule_name, char* third_rule_name, Node_kind nk, float prob){
    assert(nk & NK_TRIPLE);

    Rule* first = expect_rule(first_rule_name);
    Rule* second = expect_rule(second_rule_name);
    Rule* third = expect_rule(third_rule_name);

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
            printf("%s ::= ", g.rule[i].name);
            print_branches(g.rule[i]);
            printf("\n");
        }
    }

    printf("\n");
}


Branch* get_current_branch(Rule* rule, int depth){

    if((depth >= 0) || (rule->rk & RK_TERMINAL)){

        Branch* current_branch = rule->branch;

        float branch_prob = randrange(0, 1); 
        float cummulative_prob = current_branch->prob;

        for(size_t i = 1; i < rule->used; ++i){
            if(branch_prob <= cummulative_prob){ break; }
            
            current_branch = rule->branch + i;
            cummulative_prob += current_branch->prob;
        }

        return current_branch;

    } else {
        return get_current_branch(g.terminal_rule, 0);
    }
}

/// @brief Given an entry point, generate AST based on grammar
/// @param rule 
/// @param depth 
/// @return 
size_t generate_ast(Rule* rule, int depth){

    Branch* b = get_current_branch(rule, depth);

    switch (b->kind){
        case BK_NO_RULE:

            if(b->node_kind == NK_NUMBER){
                return node_number(randrange(-1, 1));
            } else if (b->node_kind == NK_X){
                return node_x;
            } else if(b->node_kind == NK_Y) {
                return node_y;
            } else {
                printf("Rule A should only produce terminal nodes (number, x, y)!\n");
                exit(-1);
            }

        case BK_SINGLE_RULE_NODE: {
            assert(b->node_kind & NK_UNOP);

            size_t node = generate_ast(b->next_rule.one_rule, depth - 1);

            return node_unop(b->node_kind, node);
        }

        case BK_SINGLE_RULE :
            return generate_ast(b->next_rule.one_rule, depth - 1);

        case BK_DOUBLE_RULE:
            assert(b->node_kind & NK_BINOP);

            size_t lhs = generate_ast(b->next_rule.two_rules.lhs, depth - 1);
            size_t rhs = generate_ast(b->next_rule.two_rules.rhs, depth - 1);

            return node_binop(b->node_kind, lhs, rhs);

        case BK_TRIPLE_RULE:
            assert(b->node_kind & NK_TRIPLE);
            
            size_t first = generate_ast(b->next_rule.three_rules.first, depth - 1);
            size_t second = generate_ast(b->next_rule.three_rules.second, depth - 1);
            size_t third = generate_ast(b->next_rule.three_rules.third, depth - 1);

            return node_triple(b->node_kind, first, second, third);

        default:
            printf("This rule does not exist!\n");
            exit(-1);

    }
}

#define add_normal_rule_to_grammar(name) add_rule_to_grammar(name, RK_NORMAL) // most rules won't be terminal or entry points so there's a macro for normal

void grammar(){

    init_grammar(N_RULES);

    add_rule_to_grammar("E", RK_ENTRY);
    add_rule_to_grammar("A", RK_TERMINAL);
    add_normal_rule_to_grammar("C");

    init_branches(MAX_BRANCHES);

    assert(g.entry_point != NULL); // entry point must be defined
    assert(g.terminal_rule != NULL); // terminal rule must be defined

    add_branch_to_rule("E", branch_triple_rule("C", "C", "C", NK_E, 1));

    add_branch_to_rule("A", branch_no_rule(NK_NUMBER, 1.0/3.0));
    add_branch_to_rule("A", branch_no_rule(NK_X, 1.0/3.0));
    add_branch_to_rule("A", branch_no_rule(NK_Y, 1.0/3.0));

    add_branch_to_rule("C", branch_single_rule("A", 0.1));
    add_branch_to_rule("C", branch_double_rule("C", "C", NK_ADD, 0.45));
    add_branch_to_rule("C", branch_single_rule_node("C", NK_COS, 0.45));
}

#endif

