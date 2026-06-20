# libcco

A C implementation of the **CCO** (CNT Configuration Object) language — a human-readable, lightweight configuration language designed for clarity, tooling efficiency, and expressive power.

> The project is currently under development. All bugs related to the project can be reported by submitting issues on GitHub, and we will regularly fix the reported problems

```
$temp.Endpoint: (
    url: String,
    method: HttpMethod = GET
),

api: #Endpoint("https://api.example.com", GET)
```

| Dimension | JSON | YAML | TOML | HCL | **CCO** |
|---|---|---|---|---|---|
| Human Handwriting Friendly | △ | ✅ (but indentation-sensitive) | ✅ | ✅ | ✅ Block-like, natural comments |
| No Indentation Ambiguity/Injection Risk | ✅ | ⚠️ | ✅ | ✅ | ✅ |
| "Modeling" capability (type aliases/templates/inheritance) | ✅ (convention only) | ❌ | ❌ | ✅ | ✅ Built-in |
| Pure C / Zero dependencies / Embedding-friendly | ❌ | ❌ | ❌ | ⚠️ (libucl/hcl ecosystem leans Go) | ✅ Single lib, C11 |
| Expressions / Derived values | ❌ | ❌ | ❌ | Partial | ✅ Arithmetic/comparison/logical/coalesce |

## Features

- **DOM API** — build, query, and serialize .cco values programmatically
- **Parser** — full recursive-descent parser for the CCO language spec v0.1.0
- **Type system** — primitive types, type aliases (`$typedef`), enumerations (`$enum`)
- **Templates** — classes with fields, inheritance (`+ Parent`), constructors (`$function.@`)
- **Static methods** — generic static methods, private methods, dot-notation calls
- **Expressions** — arithmetic, comparison, logical, coalescing operators
- **Built-in functions** — `$format` (parse a string as CCO at runtime)
- **Serialization** — compact and pretty-print output, file I/O
- **Diagnostics** — structured errors with source location and context
- **Zero external dependencies** — only the C standard library

## Quick Start

```c
#include <cnt/cco.h>
#include <stdio.h>

int main(void) {
    const char *src = "name: \"libcco\", version: 1";

    cco_object_t *cfg = cco_parse(src);
    if (!cfg) {
        fprintf(stderr, "Error: %s\n", cco_last_error());
        return 1;
    }

    printf("name = %s\n", cco_string_get(cco_object_get(cfg, "name")));
    printf("version = %lld\n", cco_int_get(cco_object_get(cfg, "version")));

    char *text = cco_serialize_pretty(cfg, 2);
    printf("Config:\n%s\n", text);
    free(text);

    cco_object_free(cfg);
    return 0;
}
```

## Building

### Requirements

- C11 compiler (MSVC, GCC, Clang)
- CMake >= 3.15

### Build

```sh
cmake -B target
cmake --build target
```

The library `cco.lib` is produced in `target/Debug/`.

### With tests and examples

```sh
cmake -B target -DBUILD_TESTING=ON -DCCO_BUILD_EXAMPLES=ON
cmake --build target
ctest --test-dir target -C Debug
```

## Tutorial

A step-by-step CCO syntax tutorial for beginners is available at **[learn/index.md](learn/index.md)** (23 chapters, English).

## Language Overview

CCO files contain declarations and a root value, separated by commas:

```
$typedef.Url: String,                    # type alias
$enum.Mode = (DEV, STAGING, PROD),       # enumeration
$temp.Server: (                          # template
    host: String,
    port: Integer = 8080
),
app: #Server("localhost")                 # root value with instantiation
```

### Value Types

| Type | Example | C API |
|------|---------|-------|
| None | `None` | `cco_none_new()` |
| Boolean | `true`, `false` | `cco_bool_new(1)` |
| Integer | `42`, `0xFF`, `0o10`, `0b1010` | `cco_int_new(42)` |
| Float | `3.14`, `1e10` | `cco_float_new(3.14)` |
| String | `"hello\nworld"`, `` `raw` `` | `cco_string_new("hello")` |
| Map | `(x: 1, y: 2)` | `cco_object_new()` |
| Array | `(1, 2, 3)` | `cco_array_new()` + `cco_array_wrap()` |
| Template | `#Point(0, 0)` | via instantiation |

## API Overview

### Value Layer

```c
// Construction
cco_object_t* cco_object_new(void);           // empty map
cco_object_t* cco_none_new(void);
cco_object_t* cco_string_new(const char *s);
cco_object_t* cco_int_new(long long i);
cco_object_t* cco_float_new(double f);
cco_object_t* cco_bool_new(int val);
cco_object_t* cco_array_wrap(cco_array_t *arr);

// Map operations
int           cco_object_set(cco_object_t *obj, const char *key, cco_object_t *value);
cco_object_t* cco_object_get(const cco_object_t *obj, const char *key);
int           cco_object_has_key(const cco_object_t *obj, const char *key);
int           cco_object_del(cco_object_t *obj, const char *key);
size_t        cco_object_size(const cco_object_t *obj);

// Array operations
int           cco_array_add(cco_array_t *arr, cco_object_t *value);
cco_object_t* cco_array_get(const cco_array_t *arr, size_t index);
size_t        cco_array_size(const cco_array_t *arr);

// Serialization
char*         cco_serialize(const cco_object_t *obj);
char*         cco_serialize_pretty(const cco_object_t *obj, int indent);
int           cco_save_file(const cco_object_t *obj, const char *filename);
```

### Parsing

```c
// Simple value parsing (ignores declarations)
cco_object_t* cco_parse(const char *text);
cco_object_t* cco_parse_from_file(const char *filename);

// Full language parsing (declarations + root value)
cco_parse_result_t* cco_parse_full(const char *text);
cco_parse_result_t* cco_parse_full_from_file(const char *filename);

// Inspect parse result
cco_symbol_table_t* cco_parse_result_symbols(cco_parse_result_t *res);
cco_object_t*       cco_parse_result_root(cco_parse_result_t *res);
void                cco_parse_result_free(cco_parse_result_t *res);
```

### Expressions

```c
// Build expression trees programmatically
cco_expr_t* cco_expr_literal(cco_object_t *val);
cco_expr_t* cco_expr_identifier(const char *name);
cco_expr_t* cco_expr_binary(cco_expr_t *lhs, cco_op_t op, cco_expr_t *rhs);
cco_expr_t* cco_expr_unary(cco_op_t op, cco_expr_t *operand);
cco_expr_t* cco_expr_coalesce(cco_expr_t *left, cco_expr_t *right);
cco_expr_t* cco_expr_compound(cco_expr_t *inner);
cco_expr_t* cco_expr_format(cco_expr_t *string_expr);

// Evaluate
cco_object_t* cco_expr_eval(const cco_expr_t *expr,
                            cco_symbol_table_t *st,
                            cco_object_t *self, cco_object_t *locals);

// Convenience
cco_object_t* cco_instantiate(cco_symbol_table_t *st, const char *name,
                              cco_object_t **args, size_t arg_count);
```

### Diagnostics

```c
typedef struct cco_diag {
    const char *file;
    size_t      line;
    size_t      col;
    const char *message;
    int         is_error;
} cco_diag_t;

int             cco_diag_count(void);
const cco_diag_t* cco_diag_get(int i);
void            cco_diag_print(const cco_diag_t *diag, const char *source);
void            cco_diag_print_all(const char *source);
```

## Examples

### Basic Construction

```c
cco_object_t *cfg = cco_object_new();
cco_object_set(cfg, "host", cco_string_new("localhost"));
cco_object_set(cfg, "port", cco_int_new(8080));
cco_object_set(cfg, "debug", cco_bool_new(1));

cco_array_t *peers = cco_array_new();
cco_array_add(peers, cco_string_new("10.0.0.1"));
cco_array_add(peers, cco_string_new("10.0.0.2"));
cco_object_set(cfg, "peers", cco_array_wrap(peers));

cco_save_file_pretty(cfg, "config.cco", 2);
```

### Templates with Inheritance

```c
const char *src =
    "$temp.Point: ( x: Integer, y: Integer ),\n"
    "$temp.Circle + Point: ( radius: Float ),\n"
    "shapes: (\n"
    "    origin: #Point(0, 0),\n"
    "    c:      #Circle(5, 5, 2.5),\n"
    "    nc:     #Circle(.x: 10, .y: 20, .radius: 3.0)\n"
    ")\n";

cco_parse_result_t *res = cco_parse_full(src);
cco_object_t *c = cco_object_get(
    cco_object_get(cco_parse_result_root(res), "c"), "shapes");
printf("radius = %g\n", cco_float_get(cco_object_get(c, "radius")));
cco_parse_result_free(res);
```

### Expressions with Operators

```c
cco_expr_t *e = cco_expr_binary(
    cco_expr_literal(cco_int_new(10)),
    CCO_OP_ADD,
    cco_expr_binary(
        cco_expr_literal(cco_int_new(20)),
        CCO_OP_MUL,
        cco_expr_literal(cco_int_new(3))));

cco_object_t *r = cco_expr_eval(e, NULL, NULL, NULL);
// r = 70 (10 + 20 * 3)
```

## Project Structure

```
libcco/
├── include/cnt/
│   ├── cco.h          # Public API
│   └── cco_p.h        # Private internal declarations
├── src/               # 12 implementation files (~3600 lines)
├── tests/             # 8 test suites (96 tests)
├── examples/          # 5 example programs + tree_print
├── CMakeLists.txt
└── README.md
```

## License

GPL-3.0
