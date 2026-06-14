# Chapter 16: Expressions

After this chapter you will be able to use arithmetic operators in CCO expressions.

## 16.1 What Is an Expression?

So far, all values have been fixed literals:

```cco
count: 42
```

But what if `count` needs to be computed from other values? CCO's expression system handles this.

## 16.2 Writing Expressions with `$(...)`

```cco
result: $(10 + 20)
```

`$(...)` is the expression wrapper. Inside, you can use various operators, and CCO will evaluate them. The above yields `30`.

## 16.3 Arithmetic Operators

| Operator | Meaning | Example | Result |
|----------|---------|---------|--------|
| `+` | addition | `$(10 + 20)` | 30 |
| `-` | subtraction | `$(10 - 3)` | 7 |
| `*` | multiplication | `$(6 * 7)` | 42 |
| `/` | division | `$(10 / 3)` | 3 (integer) |

## 16.4 Operator Precedence

Multiplication and division take precedence over addition and subtraction (as in math):

```cco
    result: $(10 + 20 * 3)    /* NOTE: = 10 + 60 = 70 */

    result: $($(10 + 20) * 3)  /* NOTE: = 30 * 3 = 90 */

    a: $(10 + 20),          /* NOTE: 30 (integer) */
    b: $(10.5 + 20),        /* NOTE: 30.5 (float) */
    c: $(1.5 * 2)           /* NOTE: 3.0 (float) */
```

If either operand is float, the result is float.

## 16.6 Where Can Expressions Be Used?

Expressions mainly appear in:
- Template field default values
- Constructor method bodies (Chapter 19)
- Static method bodies (Chapter 20)

For evaluation, the C API (`cco_expr_eval`) is used.

## 16.7 Summary

- `$(expression)` marks an expression
- Supports `+` `-` `*` `/`
- Multiplication/division before addition/subtraction
- Nested `$(...)` changes precedence

Next chapter -- comparison and logical operators.
