# Chapter 14: Named Instantiation

After this chapter you will be able to instantiate templates using named parameters -- no need to remember field order.

## 14.1 The Pain of Positional Args

Consider this template:

```cco
$temp.Server: (
    host: String,
    port: Integer = 8080,
    timeout: Integer = 30,
    retries: Integer = 3
)
```

If you only want to set `retries = 5`, positional args force you to write everything:

```cco
s1: #Server("localhost", 8080, 30, 5)  /* NOTE: must write all 4 args */
```

## 14.2 Named Args to the Rescue!

Named args let you **specify only the fields you want**:

```cco
s1: #Server(.host: "localhost", .retries: 5),
s2: #Server(.host: "example.com", .port: 443, .retries: 10)
```

Syntax: `.field_name: value`

## 14.3 Rules

- Named args are matched **by name**, not by position
- Unspecified fields use their defaults
- Each field name must be prefixed with a dot `.`

```cco
/* NOTE: order does not matter */
s1: #Server(.retries: 5, .host: "localhost", .timeout: 60)

/* NOTE: same result */
s2: #Server(.host: "localhost", .timeout: 60, .retries: 5)
```

## 14.4 Positional vs Named

| | Positional | Named |
|---|-----------|-------|
| Syntax | `#T(v1, v2)` | `#T(.f1: v1, .f2: v2)` |
| Order | must match field order | any order |
| Readability | count positions | self-documenting |
| Skipping defaults | awkward | just omit |

For **more than 3 fields**, named instantiation is strongly recommended.

## 14.5 Summary

- `#Template(.field: value, ...)` -- named instantiation
- Unspecified fields use defaults
- Order is arbitrary, better readability

Next chapter -- template inheritance.
