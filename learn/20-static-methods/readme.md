# Chapter 20: Static Methods `$function.#`

After this chapter you will be able to define and call static methods on templates.

## 20.1 What Is a Static Method?

A static method **belongs to the template itself**, not to any instance. You call it without creating an instance -- similar to class methods in other languages.

## 20.2 Defining a Static Method

```cco
$temp.Math: (),

$function.#square<x: Integer>: Integer (
    $return<Integer>(x * x)
)
```

Syntax breakdown:

```
$function .  # MethodName  <Params>  :  ReturnType  (  Body  )
```

- `$function.#square` -- defines a static method named `square`
- `<x: Integer>` -- parameter list (same as constructors)
- `: Integer` -- return type
- `(...)` -- method body

## 20.3 Calling Static Methods

```cco
    result: #Math:square(5)      /* NOTE: result is 25 */
    result: Math:square(6)       /* NOTE: alternative syntax */
```

Two calling styles:

| Style | Example |
|-------|---------|
| `#Template:method(args)` | `#Math:square(5)` |
| `Template:method(args)` | `Math:square(6)` |

## 20.4 Statements Inside Method Bodies

Method bodies support these statements:

| Statement | Example | Purpose |
|-----------|---------|---------|
| bind | `result: Integer = x * x` | declare a local variable |
| assign | `result = x * x` | assign to local variable |
| return | `$return<Integer>(result)` | return a value |
| expr | `callSomething()` | evaluate an expression (discard result) |

```cco
$function.#compute<a: Integer, b: Integer>: Integer (
    temp: Integer = a + b,
    $return<Integer>(temp * 2)
)
```

## 20.5 The Return Statement

`$return<Type>(expression)` returns a value:

```cco
$function.#max<a: Integer, b: Integer>: Integer (
    $return<Integer>($($(a > b) | a) && $(a | b)))
)
```

## 20.6 Summary

- `$function.#Method<params>: ReturnType ( body )` defines a static method
- Called via `#Template:method(args)` or `Template:method(args)`
- Body supports bind, assign, return, and expression statements

Next chapter -- `$format`.
