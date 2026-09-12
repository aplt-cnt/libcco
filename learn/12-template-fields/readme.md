# Chapter 12: Fields and Default Values

After this chapter you will be able to write template fields with type annotations and default values.

## 12.1 The Simplest Template

```cco
$temp.Point: (
    x: Integer,
    y: Integer
)
```

Defines a "point" template with two integer fields: `x` and `y`.

## 12.2 Full Field Syntax

```
field_name  :   Type   =   Default_Value
    ^         ^    ^         ^
identifier  colon type      equal + value (optional)
```

Both type and default value are optional:

```cco
$temp.Example: (
    a: String,        /* NOTE: has type, no default */
    b: Integer = 42,  /* NOTE: has type, has default */
    c: "hi"           /* NOTE: no type, has default (type inferred) */
)
```

## 12.3 Using Default Values

```cco
$temp.Server: (
    host: String,
    port: Integer = 8080,
    timeout: Integer = 30
),

/* Only pass required args */
s1: #Server("localhost"),              /* NOTE: port=8080, timeout=30 */
s2: #Server("example.com", 443),       /* NOTE: timeout=30 */
s3: #Server("test.com", 80, 60)        /* NOTE: all specified */
```

Fields with defaults can be omitted when instantiating.

## 12.4 Acceptable Types for Fields

A field's type can be:

- Primitive: `String`, `Integer`, `Float`, `Boolean`, `None`
- Enum name: `Mode`, `Color`, etc.
- Another template name: `Server`, `Endpoint`, etc.
- An alias: any name from `$typedef`

```cco
$typedef.Url: String,
$enum.Mode = (DEV, PROD),

$temp.Endpoint: (
    url: Url,
    mode: Mode
)
```

## 12.5 Summary

- Field format: `name: Type = Default`
- Type is optional but recommended
- Default values are optional but convenient
- Types can be primitives, enums, aliases, or templates

Next chapter -- instantiating templates.
