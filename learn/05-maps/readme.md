# Chapter 5: Maps -- Key-Value Pairs

After this chapter you will be able to group related data using CCO's maps.

## 5.1 Remember Chapter 2's Syntax?

```cco
name: "libcco",
version: 1
```

This is actually a shorthand (detailed in Chapter 7). The full form is:

```cco
(name: "libcco", version: 1)
```

Wrapped in parentheses `()`, with `key: value` pairs separated by commas -- this is CCO's **map** (also called a dictionary or object).

## 5.2 What a Map Looks Like

```cco
/* NOTE: a map containing person information */
person: (
    name: "Alice",
    age: 30,
    city: "Beijing"
)
```

Outer parentheses `(...)` enclose comma-separated key-value pairs.

## 5.3 Empty Map

An empty `()` is an empty map:

```cco
empty_object: ()
```

## 5.4 Nested Maps

A map's values can themselves be maps:

```cco
config: (
    database: (
        host: "localhost",
        port: 5432
    ),
    cache: (
        host: "redis.local",
        port: 6379
    )
)
```

## 5.5 Values Can Be Any Type

Each value in a map can be:
- A primitive (number, string, bool, None)
- Another map
- An array (next chapter)
- A template instance (covered later)

```cco
mixed: (
    a: 42,                    /* NOTE: integer */
    b: "hello",               /* NOTE: string */
    c: true,                  /* NOTE: boolean */
    d: None,                  /* NOTE: null */
    e: (nested: "map"),       /* NOTE: nested map */
    f: 3.14                   /* NOTE: float */
)
```

## 5.6 Common Mistakes

```cco
/* NOTE: missing comma between pairs -- ERROR */
(name: "Alice" age: 30)

/* NOTE: missing key name -- ERROR */
(: 42)

/* NOTE: using = instead of : -- ERROR */
(name = "Alice")
```

```cco
/* NOTE: correct */
(name: "Alice", age: 30)
```

## 5.7 Keys Are Identifiers

Map keys **do not need quotes** -- they are written as identifiers (letters, digits, underscores; must not start with a digit).

```cco
/* NOTE: valid keys */
(name: 1, _count: 2, myField: 3)

/* NOTE: invalid -- starts with a digit */
(1st: 1)
```

## 5.8 Summary

- Maps use `(key: value, ...)` syntax
- Empty map is `()`
- Values can be any type, maps can nest
- Keys are identifiers, no quotes needed

Next chapter -- arrays.
