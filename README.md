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
- **Parser** — full recursive-descent parser for the CCO language spec
- **Type system** — primitive types, type aliases (`$typedef`), enumerations (`$enum`)
- **Templates** — classes with fields, inheritance (`+ Parent`), constructors (`$function.@`)
- **Static methods** — generic static methods, private methods, dot-notation calls
- **Expressions** — arithmetic, comparison, logical, coalescing operators
- **Built-in functions** — `$format` (parse a string as CCO at runtime)
- **Serialization** — compact and pretty-print output, file I/O
- **Diagnostics** — structured errors with source location and context
- **Zero external dependencies** — only the C standard library

### Compile-time Feature Flags

Several capabilities can be trimmed at compile time. All seven flags default
to a conservative baseline; disabled syntax produces `CCO_ERR_FORBIDDEN`
rather than a silent fallback. See [docs/feature-flags.md](docs/feature-flags.md)
for the full matrix.

**ABI consistency**: the caller's translation units and the library itself MUST
be compiled with the same `CCO_ENABLE_*` flag set. Different flag combinations
produce a different `cco_parse_options_t` layout; mixing them causes ABI
mismatch. Declare your flags once in your build system and propagate them to
every consumer.

## Quick Start

```c
#include <cnt/cco.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *src = "name: \"libcco\", version: 1";

    cco_object_t *cfg = cco_parse(src);
    if (!cfg) {
        cco_diag_print_all(stderr);
        return 1;
    }

    printf("name = %s\n", cco_string_get(cco_object_get(cfg, "name")));
    printf("version = %lld\n",
           (long long)cco_int_get(cco_object_get(cfg, "version")));

    char *text = cco_serialize_pretty(cfg, 2);
    printf("Config:\n%s\n", text);
    free(text);

    cco_object_free(cfg);
    return 0;
}
```

## Building

Requires a C11 compiler (GCC, Clang, or MSVC) and GNU Make.

You can build the project using the unified generic Makefile:

```sh
make build          # Build debug binary (alias of build.debug)
make build.release  # Compile with release flags (-O2)
make run            # Run the binary (ARGS="..." to pass arguments)
make tests          # Build and run the test suite
make lint           # Run static analysis on the sources
make docs           # Generate API documentation with Doxygen
make install        # Install artifacts under /usr/local
make clean          # Remove all build artifacts
```

### Command Line Options

You can pass arbitrary compiler options and feature macros directly to `make` without modifying the Makefile. The Makefile captures all appended arguments:

* **`-co, --compile-option`**
  Pass custom compilation flags (e.g., optimization or warning flags).
* **`-fo, --feature-option`**
  Enable specific feature macros (automatically prefixed with `-D`).
* **`-h, --help`** or **`help`**
  Show the interactive help documentation.

**Examples:**
```sh
# Build with O3 optimization and LTO
make build.release -- -co -O3 -co -flto

# Build with a specific feature enabled
make build.debug -- -fo CCO_ENABLE_EVAL -fo CCO_ENABLE_FORMAT

# Show help (you can also just run 'make help')
make -- -h
```

## Tutorial

A step-by-step CCO syntax tutorial for beginners is available at
**[learn/index.md](learn/index.md)**.

## Language Overview

CCO files contain declarations and a root value, separated by commas:

```
$typedef.Url: String,                    # type alias
$enum.Mode = (DEV, STAGING, PROD),       # enumeration
$temp.Server: (                          # template
    host: String,
    port: Integer = 8080
),
app: #Server("localhost")                # root value with instantiation
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

## API Reference

The complete public API reference lives in
**[docs/api.md](docs/api.md)**. **Every PR that adds, removes, or changes a
public API symbol MUST update that file in the same commit** — CI enforces
this via `scripts/check-api-doc.sh`.

Version 0.x: the public API may change between minor versions; the API is
Doxygen-documented in `include/cnt/cco.h`. Thread safety: diagnostics and
error state are thread-local; other API calls are safe to use concurrently
on distinct objects.

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
cco_object_t *shapes = cco_object_get(cco_parse_result_root(res), "shapes");
cco_object_t *c = cco_object_get(shapes, "c");
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
│   ├── cco.h          # Public C API
│   └── cco.hpp        # C++ RAII wrapper
├── src/
│   ├── *.c/*.h        # Implementation
│   └── internal/      # Private headers (not installed)
├── tests/
│   ├── unit/
│   ├── integration/
│   └── fuzz/
├── examples/
├── benchmarks/
├── docs/
│   ├── api.md         # Authoritative API reference
│   ├── options.md
│   └── feature-flags.md
├── learn/             # Syntax tutorial
├── adr/               # Architecture Decision Records
├── scripts/
└── CMakeLists.txt
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Every commit requires a DCO
sign-off (`git commit -s`).

## License

Apache-2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE).