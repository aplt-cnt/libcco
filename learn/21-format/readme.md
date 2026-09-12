# Chapter 21: `$format` Function

After this chapter you will be able to parse strings as CCO values at runtime using `$format`.

## 21.1 The Problem

Sometimes you need to parse a text string as a CCO value at runtime. For example, a config contains the string `"(1, 2, 3)"` but you want it as an actual array.

## 21.2 Basic Usage

```cco
result: $format("(1, 2, 3)")
```

`$format(string)` parses the string as CCO code and returns the resulting value. The example above produces a three-element array.

## 21.3 Practical Example

```cco
/* NOTE: stored as a raw string */
raw: "(1, 2, 3)",

/* NOTE: parsed into a real array */
    parsed: $format(raw)     /* NOTE: result is an array containing 1, 2, 3 */
```

## 21.4 What Can Be Parsed?

`$format` can parse any valid CCO value expression:

- Primitives: `$format("42")` -> integer 42
- Strings: `$format('"hello"')` -> string "hello"
- Maps: `$format('(x: 1, y: 2)')` -> map
- Arrays: `$format("(1, 2, 3)")` -> array
- Template instances: `$format('#Point(0, 0)')` -> template instance

## 21.5 Notes

- The argument must be a string (or an expression that evaluates to a string)
- Parsing uses the current symbol table (so defined templates and enums are available)
- If the string is invalid CCO, an error is returned

## 21.6 Summary

- `$format(string)` parses CCO text at runtime
- Can parse any valid CCO value
- Useful for dynamic configuration

Next chapter -- appendix: complete grammar reference.
