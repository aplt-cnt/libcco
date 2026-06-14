# Chapter 23: Keywords & Operator Precedence

## 23.1 Complete Keyword List

### Primitive Type Keywords

| Keyword | Meaning |
|---------|---------|
| `String` | string type |
| `Integer` | integer type |
| `Float` | float type |
| `Boolean` | boolean type |
| `None` | none/null type |
| `Array` | array type |
| `dyn` | dynamic type (polymorphism) |

### Literal Keywords

| Keyword | Meaning |
|---------|---------|
| `true` | boolean true |
| `false` | boolean false |
| `None` | null value |

### Dollar Keywords

| Keyword | Chapter | Purpose |
|---------|---------|---------|
| `$typedef` | 9 | define type alias |
| `$enum` | 10 | define enumeration |
| `$temp` | 11 | define template |
| `$function` | 19-20 | define constructor or static method |
| `$default` | 19 | mark default constructor |
| `$this` | 19 | reference current instance |
| `$return` | 20 | return from method |
| `$format` | 21 | parse string as CCO at runtime |

### Other Keywords

| Keyword | Meaning |
|---------|---------|
| `this` | current instance reference (expression context) |

## 23.2 Operator Precedence (High to Low)

| Precedence | Operators | Associativity | Description |
|-----------|-----------|---------------|-------------|
| 1 (highest) | `!` | right | logical NOT (unary) |
| 2 | `*` `/` | left | multiply, divide |
| 3 | `+` `-` | left | add, subtract |
| 4 | `==` `!=` `<` `>` `<=` `>=` | left | comparison |
| 5 | `&&` | left | logical AND |
| 6 | `\|\|` | left | logical OR |
| 7 (lowest) | `\|` | left | coalesce |

## 23.3 Operator Quick Reference

| Operator | Prec | Meaning | Input Types | Result Type |
|----------|------|---------|-------------|-------------|
| `!x` | 1 | logical NOT | Boolean | Boolean |
| `a * b` | 2 | multiplication | Number | Number |
| `a / b` | 2 | division (integer truncation) | Number | Number |
| `a + b` | 3 | addition | Number | Number |
| `a - b` | 3 | subtraction | Number | Number |
| `a == b` | 4 | equal | Number | Boolean |
| `a != b` | 4 | not equal | Number | Boolean |
| `a < b` | 4 | less than | Number | Boolean |
| `a > b` | 4 | greater than | Number | Boolean |
| `a <= b` | 4 | less or equal | Number | Boolean |
| `a >= b` | 4 | greater or equal | Number | Boolean |
| `a && b` | 5 | logical AND | Boolean | Boolean |
| `a \|\| b` | 6 | logical OR | Boolean | Boolean |
| `a \| b` | 7 | coalesce | Any | Any |

## 23.4 Special Symbols

| Symbol | Name | Purpose |
|--------|------|---------|
| `(...)` | parentheses | map / array / grouping |
| `:` | colon | key-value separator / type annotation |
| `,` | comma | element separator |
| `.` | dot | separator in declarations |
| `+` | plus | addition / template inheritance |
| `#` | hash | template instantiation |
| `"..."` | double quote | string literal |
| `` `...` `` | backtick | raw string literal |
| `$` | dollar | keyword prefix |
| `@` | at | constructor marker |
| `/* */` | comment | block comment |
| `_$0` | anon param | unnamed parameter in constructor/method |
| `!` | exclamation | logical NOT / private method marker |
