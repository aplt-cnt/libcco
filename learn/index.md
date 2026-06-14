# CCO Syntax Tutorial

Welcome! This is a beginner-friendly tutorial for the CCO language.

## What is CCO?

CCO (CNT Configuration Object) is a **human-readable configuration file format**. It is more expressive than JSON, simpler than YAML, and supports type aliases, enumerations, templates (classes), inheritance, and expressions.

> Start with Chapter 1 if you are new to CCO.

## Learning Path

The tutorial is organized into **6 parts, 23 chapters**. Reading in order is recommended.

### Part 1: Meet CCO

| # | Chapter | After this you can... |
|---|---------|---------------------|
| [**1**](01-intro/readme.md) | [What is CCO?](01-intro/readme.md) | Understand what CCO is and how it compares to JSON/YAML |
| [**2**](02-first-file/readme.md) | [Your First .cco File](02-first-file/readme.md) | Write the simplest `.cco` file, understand `key: value` |
| [**3**](03-basic-data/readme.md) | [Basic Data Types](03-basic-data/readme.md) | Use None, booleans, integers (with bases), floats, strings |
| [**4**](04-comments/readme.md) | [Comments](04-comments/readme.md) | Annotate your config with `/* */` |

### Part 2: Organizing Data

| # | Chapter | After this you can... |
|---|---------|---------------------|
| [**5**](05-maps/readme.md) | [Maps -- Key-Value Pairs](05-maps/readme.md) | Group related data with `(k: v, ...)` |
| [**6**](06-arrays/readme.md) | [Arrays -- Lists](06-arrays/readme.md) | Store ordered lists with `(v, v, ...)` |
| [**7**](07-shorthand/readme.md) | [Top-Level Shorthand](07-shorthand/readme.md) | Understand the overall `.cco` file structure |

### Part 3: Making Data Clearer

| # | Chapter | After this you can... |
|---|---------|---------------------|
| [**8**](08-type-basics/readme.md) | [Type Basics](08-type-basics/readme.md) | Understand the five primitive types |
| [**9**](09-typedef/readme.md) | [$typedef -- Type Aliases](09-typedef/readme.md) | Give meaningful names to types |
| [**10**](10-enum/readme.md) | [$enum -- Enumerations](10-enum/readme.md) | Define a fixed set of allowed values |

### Part 4: Templates -- Reusable Structures

| # | Chapter | After this you can... |
|---|---------|---------------------|
| [**11**](11-template/readme.md) | [What is a Template?](11-template/readme.md) | Understand what problem templates solve |
| [**12**](12-template-fields/readme.md) | [Fields and Default Values](12-template-fields/readme.md) | Write template fields with types and defaults |
| [**13**](13-instantiation/readme.md) | [Instantiation -- Positional Args](13-instantiation/readme.md) | Create instances by passing args in order |
| [**14**](14-named-inst/readme.md) | [Named Instantiation](14-named-inst/readme.md) | Pass args by name (clearer, more flexible) |
| [**15**](15-inheritance/readme.md) | [Template Inheritance](15-inheritance/readme.md) | Extend existing templates with new fields |

### Part 5: Advanced Features

| # | Chapter | After this you can... |
|---|---------|---------------------|
| [**16**](16-expressions/readme.md) | [Expressions](16-expressions/readme.md) | Do arithmetic in CCO |
| [**17**](17-comparison-logic/readme.md) | [Comparison & Logic](17-comparison-logic/readme.md) | Use comparison and logical operators |
| [**18**](18-coalesce/readme.md) | [Coalescing Operator \|](18-coalesce/readme.md) | Handle "use default if missing" scenarios |
| [**19**](19-constructors/readme.md) | [Constructors `$function.@`](19-constructors/readme.md) | Custom template initialization logic |
| [**20**](20-static-methods/readme.md) | [Static Methods `$function.#`](20-static-methods/readme.md) | Define and call methods without instances |
| [**21**](21-format/readme.md) | [`$format` Function](21-format/readme.md) | Parse strings as CCO at runtime |

### Part 6: Appendix

| # | Chapter | Content |
|---|---------|---------|
| [**22**](22-grammar/readme.md) | [Grammar Reference](22-grammar/readme.md) | Complete EBNF-style grammar |
| [**23**](23-keywords/readme.md) | [Keywords & Precedence](23-keywords/readme.md) | Full keyword list, operator precedence table |

## Complete Examples

The [learn/source/](source/) directory contains complete `.cco` files corresponding to each chapter. All can be parsed with `cco_parse_full()`.

## Prerequisites

- No software installation needed to read the tutorial
- Running the examples requires the libcco library (see `Readme.md`)
