# WASM interop

It should be possible to construct a _prelude_ by defining functions whose body include WASM they compile to directly. These functions can then be called as others, providing a foundation from which to bootrap programs in the language. This is a type of "escape hatch" to raw WASM.

There are some possibilities for how this could be achieved syntactically. One possibility is to introduce the concept of a "named quote":

```
(fun +
  (x i32 y i32 i32)
  wasm'(local.get $x
        local.get $y
        i32.add))

(fun inc
  (x i32 i32)
  (+ 1 x))
```

A benefit of this approach is that the parsing step does not need to be aware of tokens from WASM. E.g., `$x` in the above will not be something that can trip up the parser if the `$name` syntax later is used in the higher level language itself.

Alternatively, we can just use s-expressions normally:

```
(fun +
  (x i32 y i32 i32)
  (wasm local.get $x
        local.get $y
        i32.add))

(fun inc
  (x i32 i32)
  (+ 1 x))
```

This is more "Lispy", and perhaps the parsing of our program into an AST won't present any difficulties.

## Reference WAT

```
(module
  (func $+ (export "+") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add)
  (func $inc (export "inc") (param i32) (result i32)
  	local.get 0
    i32.const 1
    call $+))
```
