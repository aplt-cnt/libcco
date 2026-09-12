# Chapter 4: Comments

After this chapter you will be able to annotate `.cco` files with explanations.

## 4.1 CCO Comments

CCO supports one comment style: **C-style block comments** `/* ... */`.

```cco
/* NOTE: this is a comment */
name: "world"
```

Comments can span multiple lines:

```cco
/*
 * A longer explanation.
 * Describes the purpose of the config below.
 */
server: "localhost",
port: 8080
```

Comments can appear between values:

```cco
name: "libcco",   /* NOTE: application name */
version: 0.1      /* NOTE: version number */
```

## 4.2 Comparison with JSON

```json
{
    /* NOTE: JSON does NOT support comments! */
    "name": "libcco"
}
```

JSON does not support comments. YAML supports `#` line comments. INI supports `;` and `#`. CCO uses `/* */`, just like C.

## 4.3 Comparison with YAML

```yaml
# YAML uses this kind of comment
name: libcco
```

CCO does not use `#` for comments because `#` has another purpose in CCO (template instantiation, covered later).

## 4.4 Tips

- Comments can appear wherever whitespace is allowed
- Comments **cannot nest** (`/* /* */ */` will error)
- Use comments to explain **why**, not **what** (the code already says what)

## 4.5 Summary

- Start with `/*`, end with `*/`
- Can span multiple lines
- Line comments (`//`) are **not** supported

Next chapter -- maps (key-value pairs).
