# Random art

<p align="center" width="100%">
    <img width="15%" src="images/logo.png">

Insipired by [Tsoding](https://www.youtube.com/watch?v=3D_h2RE0o0E).
Original paper [here](http://users.ece.cmu.edu/~adrian/projects/validation/validation.pdf).

## Premise

We can create an image by using a function that maps the (x, y) coordinates of the pixel to an (r, g, b) value. If we use the function `f(x, y) -> (x, x, x)` we get a greyscale image:

![greyscale](images/greyscale.png)

x and y are mapped to [-1, 1]. The results from the function are then scaled to map to [0,255] for each channel.

What if we could generate the functiion `f` randomly, then evaluate it on the (x, y) input !?

In the context of security, we can use for example an SSH key hash as the seed for RNG before generating the function, in effect, visualising the hash. 

## Some of my favourite images

These were generated by letting the grammar generate a function to a certain depth.

| Depth3 | Depth 6 | Depth 1 |
|--------|--------|-------|
| ![1](images/31.png) | ![2](images/61.png) | ![3](images/11.png) |
| ![1](images/32.png) | ![2](images/62.png) | ![3](images/12.png) |
| ![1](images/33.png) | ![2](images/63.png) | ![3](images/13.png) |


All the images so far use this grammar given in the paper:

![g](images/grammar.png)

## Extending the grammar

- [ ] `ifgeq-then-else`
- [ ] `mod`
- [ ] `div`
- [ ] `sin`
- [ ] `cos`
- [ ] `exp`

There's code to interpret these additional constructs, so it is possible to write ASTs by hand that use them. They haven't been added to the grammar. The grammar should be easily extendable, for example, the paper grammar is represented like so:
```C
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
```

These are some images produces from ASTs that implement these operators:

| Depth 2 | Depth 5 | Depth 13 |
|--------|--------|-------|
| ![c2](images/c2.png) | ![c5](images/c5.png) | ![c41](images/c41.png) |
| `E(y,add(x, x) >= div(y, y),y)` | Function too big to show here | Function too big to show here |

## Todo
- [ ] Exploration

**Note:** Nesting depth is currently limited to 8. Higher numbers cause weird issues, but we don't need massive functions anyway. They may noy actually be that useful.
