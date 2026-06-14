# Chapter 3: Basic Data Types

After this chapter you will be able to write all of CCO's primitive values.

## 3.1 None (Null)

Equivalent to `null` or `nil` in other languages:

```cco
nothing: None
```

The keyword `None` (capitalized) means "no value".

## 3.2 Boolean (True/False)

Only two values:

```cco
debug: true,
production: false
```

The keywords `true` and `false` are lowercase.

## 3.3 Integers

Plain integers:

```cco
count: 42,
negative: -10
```

CCO also supports other bases and digit separators:

| Writing | Meaning | Actual Value |
|---------|---------|--------------|
| `42` | decimal | 42 |
| `0xFF` | hexadecimal (`0x` prefix) | 255 |
| `0o10` | octal (`0o` prefix) | 8 |
| `0b1010` | binary (`0b` prefix) | 10 |
| `1_000_000` | underscore separator | 1000000 |

Underscores `_` are for readability only:

```cco
large_number: 1_000_000_000
```

## 3.4 Floats

Numbers with a decimal point:

```cco
pi: 3.14,
avogadro: 6.02e23,
small: 1.0
```

Scientific notation via `e` (or `E`) is supported:

```cco
price: 1.99e10
```

## 3.5 Strings

Wrapped in double quotes:

```cco
greeting: "Hello, world!"
```

Strings support escape sequences:

| Sequence | Meaning |
|----------|---------|
| `\n` | newline |
| `\t` | tab |
| `\\` | backslash |
| `\"` | double quote |
| `\r` | carriage return |
| `\xNN` | hex byte (e.g. `\x41` = 'A') |
| `\uNNNN` | Unicode character (e.g. `\u0041` = 'A') |

```cco
message: "line one\nline two",
path: "C:\\Program Files\\app"
```

### Raw Strings (Backticks)

If you want to avoid escape processing, use backticks `` ` ``. Everything inside is literal:

```cco
regex: `\d+\.\d+`,
multiline: `first line
second line
third line`
```

Backtick strings **do not process any escapes** -- `\n` is two characters (backslash + n).

## 3.6 Summary

| Type | Example | Notes |
|------|---------|-------|
| None | `None` | null value |
| Boolean | `true`, `false` | true/false |
| Integer | `42`, `0xFF`, `0b1010`, `1_000` | multiple bases, underscores |
| Float | `3.14`, `1e10` | scientific notation |
| String | `"hello"`, `` `raw` `` | double-quote with escapes, backtick for raw |

Next chapter -- comments.
