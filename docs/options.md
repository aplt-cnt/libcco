# Parse Options

**Status: indev.** `cco_parse_options_t` sets resource ceilings, turns
individual language features on or off, and pins the sandbox root. Every
field has a default from `cco_parse_options_default()`.

The struct's layout depends on the `CCO_ENABLE_*` compile-time set.
Fields guarded by `#if CCO_ENABLE_*` exist only when the matching macro
is `1`. The caller's translation units and the library MUST be compiled
with the same set.

## Filling the struct

```c
cco_parse_options_t opts = cco_parse_options_default();
opts.max_nesting_depth = 64;
opts.strict_types = true;
cco_parse_result_t* res = cco_parse_full_with_options(src, &opts);
```

Every field is safe to leave at its default. `cco_parse_options_default`
zeroes the struct first, so an unset field is never garbage.

## Resource ceilings

Each ceiling is a hard stop. Passing it fails the parse with
`CCO_ERR_OUT_OF_RANGE` (or `CCO_ERR_DEPTH_EXCEEDED` for depth), and one
diagnostic lands in the thread-local list.

### `max_nesting_depth` (default 256)

Nesting levels of maps, arrays, and template instantiations. Guards
against stack exhaustion from `((((...))))`. Each `parse_value` entry
increments a depth counter, checked at the top of the function.

### `max_string_length` (default 16 MiB)

Byte length of a single string value after escape decoding. Strings
under this ceiling pass through; strings at or over it fail with
`CCO_ERR_OUT_OF_RANGE`. Enforced inside string construction.

### `max_document_size` (default 64 MiB)

Byte length of the whole input. Checked once at the parse entry point
after the input is read. A document that exceeds the ceiling never
reaches the lexer.

### `max_total_alloc` (default 256 MiB)

Total bytes the parse arena may hand out, cumulatively. Enforced inside
every `cco_arena_alloc` call. When the arena cannot serve the next
request, the allocation returns `NULL` and the parse unwinds.

### `max_instantiations` (default 10000)

Template instantiations in a single parse. Counts each successful
`#Name(...)` build. Exceeding the ceiling stops further instantiations
with `CCO_ERR_OUT_OF_RANGE`.

### `max_steps` (default 10_000_000)

Lexer token budget. Increments once per emitted token. Guards against
pathological inputs that would otherwise spin without bound. Exceeding
it stops the parse with `CCO_ERR_OUT_OF_RANGE`.

## Language toggles

### `allow_expressions` (default true)

When false, `$(...)` syntax is rejected at parse time with
`CCO_ERR_FORBIDDEN`. Existing expression nodes in an AST are still
serializable.

### `allow_colon_inst` (default true)

When false, `#Name: (...)` colon-instantiation is rejected at parse
time. Positional and named instantiation stay available.

### `strict_types` (default false)

When true, a value that does not match its declared field type fails
with `CCO_ERR_TYPE_MISMATCH`. When false, compatible values coerce:
`Integer` accepting a float literal that fits, `String` accepting a
raw string, and so on.

### `restrict_fs` (default true)

Gates every path the library touches, including
`cco_parse_from_file`, `cco_parse_full_from_file`, and
`cco_save_file_pretty`. With `base_dir` set, the check resolves both
paths with `realpath` and compares. With `base_dir` unset, the current
working directory and its descendants pass.

### `base_dir` (default NULL)

Sandbox root. Only consulted when `restrict_fs` is true. A rejected
path returns `CCO_ERR_FORBIDDEN`. A path `realpath` cannot resolve
returns `CCO_ERR_IO`.

## Feature-gated fields

These fields exist only when the matching compile-time switch is on.
Reading them from a translation unit built against a different switch
set is a hard ABI break.

### `allow_format` (gate `CCO_ENABLE_FORMAT`, default false)

Runtime control for `$format(...)`. With the field false, parse-time
`$format(` fails with `CCO_ERR_FORBIDDEN`. With the field true and the
switch on, the embedded string parses as CCO with the parent's options.

### `format_inherits_options` (gate `CCO_ENABLE_FORMAT`, default true)

When true, the nested parse inherits every ceiling and toggle from the
parent. When false, `options.format_options` (a sub-struct) supplies a
separate set. The two cannot be mixed: turning inheritance off without
filling the sub-struct uses defaults.

### `allow_env` (gate `CCO_ENABLE_ENV`, default false)

Runtime control for `$env("VAR")`. With the field false, parse-time
`$env(` fails with `CCO_ERR_FORBIDDEN`. With the field true and the
switch on, the call reads the environment; an unset variable yields
`None`. Environment values never appear in diagnostics.

### `allow_static_calls` (gate `CCO_ENABLE_STATIC_CALLS`, default true)

Runtime control for `#T:method(...)` and `T.method(...)`. With the field
false, both forms fail at parse time with `CCO_ERR_FORBIDDEN`.

### `allow_constructors` (gate `CCO_ENABLE_CONSTRUCTORS`, default true)

Runtime control for `$function.@`. With the field false, the syntax
fails at parse time with `CCO_ERR_FORBIDDEN`. Default constructors stay
available.

### `pretty_preserve_comments` (gate `CCO_ENABLE_COMMENT_PRESERVE`, default false)

When true, `cco_serialize_pretty` emits stored comments alongside the
values. Slices come verbatim from source. When false, comments are
dropped at serialization time even if the lexer recorded them.

### `collect_diagnostics` (gate `CCO_ENABLE_ERROR_RECOVERY`, default true)

When true and the compile-time switch is on, the parser collects up to
64 diagnostics per parse and returns the first error code. When false,
parsing stops at the first error and exactly one diagnostic lands.

## Enforcement points

| Field | Checked in |
|---|---|
| `max_document_size` | parse entry, after input read |
| `max_nesting_depth` | `parse_value` entry |
| `max_instantiations` | template instantiation counter |
| `max_total_alloc` | every arena allocation |
| `max_string_length` | string construction |
| `max_steps` | lexer, once per token |
| `restrict_fs`, `base_dir` | file read and file write paths |
| `allow_format` | parse-time `$format(` gate |
| `allow_env` | parse-time `$env(` gate |
| `allow_static_calls` | parse-time `#T:method` / `T.method` gate |
| `allow_constructors` | parse-time `$function.@` gate |
| `allow_colon_inst` | parse-time `#Name: (...)` gate |
| `pretty_preserve_comments` | `cco_serialize_pretty` comment emission |
| `collect_diagnostics` | parser error accumulation |

## Asymmetry between serialization and parse

A format node built by hand with `cco_expr_format` while
`CCO_ENABLE_FORMAT=OFF` is valid data. `cco_serialize_pretty` writes it
as `$format(...)` because serialization only looks at the AST node
type. Re-parsing that output fails with `CCO_ERR_FORBIDDEN`, because
parse sees the literal syntax. The two paths are independent and each
has its own test.
