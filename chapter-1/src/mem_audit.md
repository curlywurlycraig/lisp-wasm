List and categorisation of all `mmalloc` calls.


# Overview

Steps to evaluating an inputted list:
1. Tokenize input
   1. Outputs a TokenizeResult.
   2. Intermediate allocations should be freeable.
2. Parse input
   1. Outputs an AST (`List`). `TokenizeResult` can be deallocated.
3. Eval input
   1. Evaluation of a `List` produces `Value`s along the way.
   2. Bits of the AST can be free'd when they produce a `Value`, but some can't (e.g. storing a function in the env). Some can only be free'd after a while.
   3. Insight: concept of a stack. I actually already have a stack, but I can use it for more. Deallocate `List` when it is evalluated. Deallocate `Value`s when they are popped from the stack and used.

# `parse.c`

## tokenizer

1. TokenFinders. Can live for the lifetime of the program.
2. TokenizeResult (including tokens and raw token `char*`). This is a bit more complicated. Can any of this live on in the form of `Value*`s?
   1. Answer: the `char* name` does, but the rest can be dropped.
3. Therefore, all tokenizer step memory allocations can be dropped except from the `name`s
4. This could be simplified if I made a custom allocator for the names, and used string interning.

## parser

1. ParseInfo. Holds onto `TokenizeResult`, and raw formula.
2. `List`, `Elem`, these can live on in the form of `LispFunction`.
3. `LispFunction` should be dropped if not stored in the env.
4. `List` and `Elem` when evalled and the result is not a `LispFunction` can be dropped.
5. Therefore, `List` and `Elem` allocations can only be dropped once they have been evaluated and are not `LispFunction`s

# `interpret.c`

1. Nightmare.

# env

## `initEnv`

1. Bindings.
2. Cell values.

Both of these can live for the lifetime of the program.
Newly created bindings should not be freed.

The `Value*`s in the bindings themselves will be quite tricky, since `Value*`s are created as part of the evaluation stage.

