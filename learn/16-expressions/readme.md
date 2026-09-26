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

## 16.4 Power and Bitwise Operators

CCO also supports the power operator `**` and bitwise operators `|`, `&`, `^`, `~`:

```cco
a: $(2 ** 3),      /* NOTE: 8 */
b: $(3 | 5),       /* NOTE: bitwise OR, 7 */
c: $(~0)           /* NOTE: bitwise NOT */
```

## 16.5 Operator Precedence

Expressions are evaluated strictly according to the following precedence levels (from highest to lowest):

1. **Unary**: `+` `-` `!` `~`
2. **Power**: `**` (Right-associative)
3. **Multiplication/Division**: `*` `/`
4. **Addition/Subtraction**: `+` `-`
5. **Ranges**: `..` `..=`
6. **Coalesce**: `??`
7. **Comparison**: `<` `<=` `>` `>=`
8. **Equality**: `==` `!=`
9. **Bitwise AND**: `&`
10. **Bitwise XOR**: `^`
11. **Bitwise OR**: `|`
12. **Logical AND**: `&&`
13. **Ternary**: `? :` (Right-associative)
14. **Logical OR**: `||`

You can always use nested `$(...)` to force a specific evaluation order.

If either operand is float, the result is float.

```cco
A: $(10 + 20 * 3)    /* NOTE: = 10 + 60 = 70 */
B: $($(10 + 20) * 3) /* NOTE: = 30 * 3 = 90 */
```

## 16.6 Where Can Expressions Be Used?

Expressions mainly appear in:
- Template field default values
- Constructor method bodies (Chapter 19)
- Static method bodies (Chapter 20)

For evaluation, the C API (`cco_expr_eval`) is used.

## 16.7 Summary

- `$(expression)` marks an expression
- Supports `+` `-` `*` `/` `**` `|` `&` `^` `~`
- Multiplication/division before addition/subtraction
- Nested `$(...)` changes precedence

Next chapter -- comparison and logical operators.

