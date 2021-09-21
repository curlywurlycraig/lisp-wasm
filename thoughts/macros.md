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
