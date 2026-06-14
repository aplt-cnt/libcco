# Chapter 18: Coalescing Operator

After this chapter you will be able to handle "use default if None" scenarios gracefully.

## 18.1 The Problem

You have a config value that might be `None` (empty). You want: if it has a value, use it; if it's `None`, use a fallback.

## 18.2 The Coalescing Operator `|`

CCO uses the **single pipe `|`** as the coalescing operator:

```
left | right
```

**If the left value is not `None`, take the left; otherwise take the right.**

## 18.3 Examples

```cco
    a: $(None | 42),    /* NOTE: None is discarded, result is 42 */
    b: $(10 | 99),      /* NOTE: 10 is not None, result is 10 */
    c: $(None | None | 5)  /* NOTE: chain: result is 5 */

    port: Integer = None,    /* NOTE: default is None */

    /* NOTE: Inside a constructor: */
    /* NOTE: port | fallback_port -> use port if set, otherwise 8080 */

    a: $(true || false),    /* NOTE: -> true (boolean) */
    b: $(None | 42)         /* NOTE: -> 42 (integer) */
```

## 18.6 Summary

- `left | right`: if left is not None, use left; otherwise use right
- Can be chained: `$a | $b | $c`
- Different from `||` (logical OR)

Next chapter -- constructors.
