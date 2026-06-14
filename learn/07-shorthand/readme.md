# Chapter 7: Top-Level Shorthand

After this chapter you will understand the overall structure of a `.cco` file.

## 7.1 Review

In Chapter 2 we wrote:

```cco
name: "libcco",
version: 1
```

In Chapter 5 we said this is equivalent to a map. Strictly speaking, at the **file top level**, writing `key: value` is a **shorthand** for `(key: value)`.

## 7.2 Full Form vs Shorthand

These two are equivalent:

```cco
/* NOTE: shorthand at top level (recommended for daily use) */
name: "libcco",
version: 1

/* NOTE: full form (same result) */
(name: "libcco", version: 1)
```

**Most people use the shorthand** -- it is cleaner and more natural.

## 7.3 When Must You Use `()` ?

**The shorthand only works at the outermost file level.** Inside a nested map, you must write `(k: v, ...)`:

```cco
/* NOTE: correct */
app: (                          /* NOTE: app: is top-level shorthand */
    name: "libcco",              /* NOTE: inside a map now */
    version: 1
)

/* NOTE: ERROR -- cannot use shorthand inside a map */
app:
    name: "libcco",
    version: 1
```

## 7.4 Complete Example

```cco
/* NOTE: a user config using top-level shorthand */
name: "Alice",
age: 30,
address: (
    city: "Beijing",
    zip: "100000"
),
hobbies: (
    "reading",
    "running",
    "coding"
)
```

This file is a map at the top level, equivalent to:

```cco
(
    name: "Alice",
    age: 30,
    address: (city: "Beijing", zip: "100000"),
    hobbies: ("reading", "running", "coding")
)
```

## 7.5 Summary

- `.cco` files can omit the outer `(...)` at the top level and write `key: value` directly
- Multiple pairs are separated by commas
- Inside nested structures, you **must** use `(...)`

Now that you understand CCO's data model, the next chapters introduce the type system.

## 7.6 Exercise

Try creating a `profile.cco` file with: name, age, address (city + zip), and hobbies. Compare with `/learn/source/basic.cco`.
