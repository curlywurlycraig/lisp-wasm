Two components to focus on with strings:

1. Interning of identifiers during parsing.
2. Handling of strings in the lisp (arguments, .data section, concatenation, etc)

## Handling of strings in the lisp

Consider the following function:

```
(fun past-tense
  (verb String String)
  (cat verb "ed"))
```

This function adds the "-ed" suffix.

There is already a lot to consider with this:

1. How to store the string literal "ed".
2. How to pass `String`s as arguments.
3. How to "modify" `String`s (if they should be modified at all).
4. How to return `String`s from functions.

Here's an attempt to answer these, but some of this might change:

## 1. Storing string literals

`WAT` provides a `(data ` invocation that allows for storage of data at compile time, much like the `.data` section of an object file. The above program would compile to something like:

```
(module
  (import "js" "mem" (memory 1))
  (data (i32.const 0) "ed")
  (function $past-tense ...))
```

And if there is more than one string literal, like in:

```
(fun past-tense
  (verb String String)
  (cat verb "ed"))

(fun present-tense
  (verb String String)
  (cat verb "ing"))
```

then something like the following would be emitted:

```
(module
  (import "js" "mem" (memory 1))
  (data (i32.const 0) "ed")
  (data (i32.const 2) "ing")
  (function $past-tense ...))
  (function $present-tense ...))
```

Alternatively, all of the string literals could simply be emitted as the same `(data ...` list:

```
(module
  (import "js" "mem" (memory 1))
  (data (i32.const 0) "eding")
  (function $past-tense ...))
  (function $present-tense ...))
```

But how to actually access the literals? These would be represented as i32 values representing the offset of the data. So the body of the functions would replace the appearance of "ing" and "ed" with the indices (pointers, essentially, to data in the imported memory (heap)).

The other aspect to consider is that a `String` has some length. We could do something like Rust slices, whereby a `String` under the hood stores a pointer to the first character, and a length.

Strings are already an interesting challenge, because it requires heap allocation (and therefore freeing) some kind of pointer dereferencing logic, immutability, etc.
And also declaring data structures. Something like:

```
(struct String
  (ptr i32)
  (length i32))
```

[Multiple values returned](https://github.com/WebAssembly/proposals) from functions is not yet ready, so it would be safest in the short run to heap allocate return values that are structures (like `Box`ed values in Rust).

A good first step is to try having a function be able to return a struct (by allocating it on the heap and returning its pointer).

## Heap storage, clearing stored values, GC, etc
