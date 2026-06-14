# Chapter 9: `$typedef` -- Type Aliases

After this chapter you will be able to give meaningful names to types using `$typedef`.

## 9.1 The URL Problem

Suppose you write an API configuration:

```cco
api: (
    url: "https://example.com"
)
```

The `url` value is a string, but "URL" and "plain string" are semantically different. If you could define `url`'s type as `Url`, the config becomes clearer.

## 9.2 Defining an Alias with $typedef

```cco
$typedef.Url: String,
```

This means: **`Url` is an alias for `String`** .

Now use it:

```cco
api: (
    url: "https://example.com"
)
```

> Note: `$typedef` does not enforce type constraints at parse time -- it serves as documentation and semantic annotation.

## 9.3 Syntax Breakdown

```
$typedef .   Name   :   Type   ,
   ^      ^    ^      ^    ^
keyword   dot  name  colon  type  comma (separator)
```

- `$typedef` is the keyword
- `.` the dot (required)
- `Name` is your alias (conventionally capitalized)
- `:` colon
- `Type` can be `String`, `Integer`, `Float`, `Boolean`, `None`, or another alias

## 9.4 Multiple Aliases

```cco
$typedef.Url: String,
$typedef.Port: Integer,
$typedef.Timeout: Integer,

app: (
    url: "https://example.com",
    port: 8080,
    timeout: 30
)
```

## 9.5 Placement Rules

All `$typedef` declarations must appear at the **file top level**, before the root value:

```cco
/* NOTE: correct -- declarations first, data after */
$typedef.Url: String,
app: (url: "https://example.com")
```

```cco
/* NOTE: ERROR -- $typedef cannot appear inside a map */
app: (
    $typedef.Url: String,
    url: "https://example.com"
)
```

## 9.6 Notes

- `$typedef` declarations are separated by commas
- Alias names must not conflict with keywords or existing names
- Aliases can reference other aliases: `$typedef.SecureUrl: Url`

## 9.7 Summary

- `$typedef.Name: Type` creates a type alias
- Must appear at file top level
- Makes config semantics clearer

Next chapter -- enumerations.
