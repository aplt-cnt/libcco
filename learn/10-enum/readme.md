# Chapter 10: `$enum` -- Enumerations

After this chapter you will be able to define and use enumerations to constrain allowed values.

## 10.1 The Mode Problem

Suppose you have a server config with a "mode" -- development, staging, or production. Without enums, you might write raw strings:

```cco
server: (
    host: "localhost",
    mode: "dev"          /* NOTE: "dev", "test", "prod" -- but what if someone misspells? */
)
```

The problem: someone might write `"DEV"` or `"dev "` with an extra space. **Enumerations** solve this.

## 10.2 Defining an Enum with $enum

```cco
$enum.Mode = (DEV, STAGING, PROD),
```

Then use it:

```cco
server: (
    host: "localhost",
    mode: PROD           /* NOTE: only DEV, STAGING, or PROD allowed */
)
```

## 10.3 Syntax Breakdown

```
$enum  .  Name  =  (  VAL1  ,  VAL2  ,  ...  )  ,
  ^     ^  ^    ^    ^      ^       ^         ^
keyword dot name equal  left-paren  comma-sep  right-paren  comma
```

- Values are separated by commas
- Enum values are plain identifiers (no quotes)

## 10.4 Enum Values Are Integers Internally

Enum values are stored as integers, numbered from 0 in definition order:

```cco
$enum.Color = (RED, GREEN, BLUE),
/* RED = 0, GREEN = 1, BLUE = 2 */
```

So `mode: PROD` stores as the number `2`. But you **write the name**, not the number.

## 10.5 Multiple Enums

```cco
$enum.Mode = (DEV, STAGING, PROD),
$enum.Level = (LOW, MEDIUM, HIGH),
$enum.Color = (RED, GREEN, BLUE),

app: (
    mode: DEV,
    level: HIGH,
    color: GREEN
)
```

## 10.6 Enum vs String

```cco
/* NOTE: error-prone */
mode: "dev",

/* NOTE: parser catches typos */
mode: DEV
```

If you write `DEVZ` instead of `DEV`, the parser immediately reports an error -- no silent bugs.

## 10.7 Summary

- `$enum.Name = (VAL1, VAL2, ...)` defines an enumeration
- Enum values conventionally use UPPERCASE (not enforced)
- Values are stored as integers (starting at 0)
- Misspelled enum values produce parse errors, preventing hidden bugs

Next chapter -- templates, CCO's most powerful feature.
