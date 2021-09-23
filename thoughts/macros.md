Macros can be useful where the function indirection isn't really desirable.
For example, implementation of an if statement, whose usage looks like the following:

```
(macro if
  (cond ba bb)
  (wasm (unwasm cond)
  	if (result i32)
  	   (unwasm ba)
	else
	   (unwasm bb)
	end))

(fun <
  (x i32 y i32 i32)
  (wasm local.get $x
  	local.get $y
	i32.lt_s))

(fun lt-example
  (x i32 i32)
  (if (< 10 x)
      x
      10))


```

We can achieve the above by introducting a macro expansion step before emitting. Simply replace the macro call with the transformed AST. Note that in the above, `unwasm` means to temporarily "turn off" the wasm pass-through mechanism that the `wasm` special form provides.

## Additional thoughts

Some things that would be helpful, that are challenging with the current compilation approach:

1. `if` statements that have different return values in different branches.
2. varargs. Function definitions right now have a fixed number of arguments.

Examples:

```
(fun max
  ((first ...nums) ...i32)
  (loop (max first
         rest (first nums))

```

Possible input:

```
(macro if
  (cond i32 ba T bb U)
  (wasm if (result T)
  	   ba
	else
	   bb
	end))
```

## thoughts on varargs

It's nice to do things like:

```
(all true 1 "yes")
```

In other words, being able to treat the arguments as something like a list is very useful.

And of course a macro like:


```
(apply all [true 1 "yes"])
```

or whatever.
