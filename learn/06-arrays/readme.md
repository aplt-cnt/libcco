# Chapter 6: Arrays -- Lists

After this chapter you will be able to write arrays and understand the difference between maps and arrays.

## 6.1 Arrays Also Use `()`

Yes -- CCO arrays **also** use parentheses:

```cco
/* NOTE: an array of numbers */
numbers: (1, 2, 3, 4, 5)
```

## 6.2 How to Tell Arrays from Maps?

Here is the rule you need to remember:

> If the **first element** inside `(...)` is a `key: value` pair, it is a map. Otherwise it is an array.

| Writing | First Element | Result |
|---------|--------------|--------|
| `(x: 1, y: 2)` | `x: 1` (has colon) | map |
| `(42, "hi")` | `42` (no colon) | array |
| `()` | empty | map (default) |

## 6.3 Array Examples

```cco
empty: (),                          /* NOTE: empty -- defaults to map */
nums: (1, 2, 3),                   /* NOTE: numeric array */
mixed: (42, "hello", true, None),  /* NOTE: mixed-type array */
nested: (
    (1, 2),
    (3, 4)                         /* NOTE: 2D array */
)
```

## 6.4 Pitfalls

You might think you wrote an array, but CCO sees a map:

```cco
/* NOTE: looks like a single-element array? */
(x: 1)

/* NOTE: actually a map { x: 1 } */
```

If the first element is `ident: value`, it is treated as a map. For a single-element array, make sure the first element is **not** an `ident: value` form:

```cco
/* NOTE: single-element array */
(42)

/* NOTE: single-element array (string, not identifier) */
("hello")
```

## 6.5 Practical Examples

```cco
users: (
    "Alice",
    "Bob",
    "Charlie"
)

scores: (
    (name: "Alice", score: 95),
    (name: "Bob", score: 87),
    (name: "Charlie", score: 92)
)
```

Above, `scores` is an array where each element is a map.

## 6.6 Summary

| | Map | Array |
|---|-----|-------|
| Syntax | `(k: v, ...)` | `(v, v, ...)` |
| Elements | key-value pairs | values |
| Empty `()` | default | -- |
| First element `k: v` | yes | no |

Next chapter -- the top-level shorthand, the most common way to write `.cco` files.
