# Chapter 8: Type Basics

After this chapter you will understand the concept of "types" in CCO -- essential for `$typedef`, `$enum`, and `$temp`.

## 8.1 What Is a Type?

"Type" means "kind of data". Each value from Chapter 3 has a corresponding type:

| Value Example | Type Name | Purpose |
|--------------|-----------|---------|
| `"hello"` | `String` | text |
| `42` | `Integer` | whole numbers |
| `3.14` | `Float` | decimal numbers |
| `true` / `false` | `Boolean` | true/false |
| `None` | `None` | null/empty |

In later chapters, we use these type names to declare "what type this field should be".

## 8.2 Why Do Types Matter?

Suppose you are configuring a database. You want:

- `host` to be text (String)
- `port` to be a number (Integer)

With a type system, you can **explicitly declare** each field's type, reducing the chance of mistakes. You will see this in action when we cover `$temp`.

## 8.3 These Five Keywords Are Reserved

In CCO, these five words are reserved:

```
String    Integer    Float    Boolean    None
```

They cannot be used as variable names or field names.

## 8.4 Bonus: Array and dyn

Besides the five primitives, CCO also supports:

- `Array` -- array type (for constraining array element types)
- `dyn BaseTemplate` -- dynamic type, meaning "any instance of BaseTemplate or its subclasses"

You just need to know these exist for now. Details come later.

## 8.5 Summary

| Keyword | Meaning |
|---------|---------|
| `String` | text |
| `Integer` | integer |
| `Float` | floating-point number |
| `Boolean` | true/false |
| `None` | null |
| `Array` | array |
| `dyn` | dynamic dispatch (polymorphism) |

Next chapter -- type aliases with `$typedef`.
