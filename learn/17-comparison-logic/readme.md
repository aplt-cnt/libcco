# Chapter 17: Comparison & Logic

After this chapter you will be able to use comparison and logical operators in CCO expressions.

## 17.1 Comparison Operators

| Operator | Meaning | Example |
|----------|---------|---------|
| `==` | equal | `$(10 == 10)` -> `true` |
| `!=` | not equal | `$(10 != 5)` -> `true` |
| `<` | less than | `$(3 < 5)` -> `true` |
| `>` | greater than | `$(10 > 5)` -> `true` |
| `<=` | less or equal | `$(5 <= 5)` -> `true` |
| `>=` | greater or equal | `$(5 >= 3)` -> `true` |

Results are boolean values `true` or `false`:

```cco
    a: $(10 == 10),   /* NOTE: true */
    b: $(10 != 5),    /* NOTE: true */
    c: $(3 > 10)      /* NOTE: false */

    both:  $(true && true),       /* NOTE: true */
    either: $(true || false),      /* NOTE: true */
    not_it: $(!false)              /* NOTE: true */

    result: $($(10 > 5) && $(3 < 10)),   /* NOTE: true && true = true */
    result: $($(10 == 10) || $(5 == 3))  /* NOTE: true || false = true */
```

## 17.4 Common Mistakes

```cco
/* NOTE: == for equality, not = */
a: $(10 = 10)    /* ERROR */

/* NOTE: string comparison with operators is limited */
a: $("hello" == "world")   /* may not work as expected */
```

> Current CCO comparison operators mainly support numeric types (Integer and Float).

## 17.5 Summary

- Comparison: `==` `!=` `<` `>` `<=` `>=`
- Logical: `&&` `||` `!`
- Results are always boolean

Next chapter -- the coalescing operator.
