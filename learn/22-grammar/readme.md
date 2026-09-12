# Chapter 22: Complete Grammar Reference

This is the full CCO grammar specification in EBNF-like notation. Useful for quick reference.

## 22.1 File Structure

```
File       ::= (Declaration ",")* RootValue ","?
RootValue  ::= Map | Array | Literal | TemplateInstance
```

**Parsing note (root-less files):** `cco_parse_full()` requires a `RootValue`
(else: `"no root expression in .cco file"`). The multi-context entry points
`cco_parse_full_froms(text, ...)` and `cco_parse_full_from_files(path, ...)`
accept **declaration-only input**: with zero (or no) root values they return a
valid result whose `root` is `NULL` and whose symbol table holds every declared
type/function. A schema-only file such as `search/openamethyst.system.cco`
parses fine through these entry points (parser.c:1833-1861).

## 22.2 Values

```
Value      ::= Map | Array | Literal
             | TemplateInstance | StaticCall
             | EnumConstant | Identifier
             | FunctionCall

Map        ::= "(" Field ("," Field)* ","? ")"
             | Identifier ":" Value           (top-level shorthand, file root only)
Field      ::= Identifier ":" Value

Array      ::= "(" Value ("," Value)* ","? ")"
Empty "()" ::= treated as empty Map

FunctionCall ::= "$" Identifier "(" Args ")"   (value-layer call; builtin "env")
```

**Disambiguation**: If the first element of `(...)` is `Identifier : Value`, it is a Map; otherwise an Array.

## 22.3 Literals

```
Literal    ::= String | RawString | Integer | Float
             | "true" | "false" | "None"

String     ::= '"' (char | escape)* '"'
RawString  ::= '`' (any char)* '`'
Integer    ::= decimal | "0x" hex | "0o" octal | "0b" binary
               Underscores "_" allowed between digits
Float      ::= digits "." digits [exponent]
             | digits exponent
             | integer "." integer
             Underscores "_" allowed between digits
```

## 22.4 Declarations

```
Declaration ::= TypeAlias | Enum | Template | Function

TypeAlias  ::= "$typedef" "." Identifier ":" TypeExpr
Enum       ::= "$enum" "." Identifier "=" "(" Identifier ("," Identifier)* ","? ")"
Template   ::= "$temp" ["<" GenericParam ">"] "." Name ["+" ParentName] ":" "(" TemplateBodyItem* ")"

Function   ::= StandaloneFunction | AttachedConstructor

StandaloneFunction
           ::= "$function" "." ["!"] "#" FunctionName Signature [Body]
AttachedConstructor
           ::= "$function" "." "@" ["<" ParamList ">"] ":" "(" Body ")"
             | "$function" "." "@" ":" "$default"     (in-template only, see 22.5)
```

Template names are **dotted**: `$temp.Provider.OpenAI:` declares template
`Provider.OpenAI`; the parent is dotted too, `$temp.MyTool + Tool.Function:`
(parser.c:1526, 1534). Standalone functions use the `#` form with the same
signature sugar as methods (parser.c:1256-1290); a missing `Body` is allowed
(bodyless, e.g. `$function.#env: (name: String) -> String`). The top-level
constructor attach `$function.@<Params>: (Body)` attaches to the most recently
declared template (deferred via `flush_pending_ctors`, parser.c:1221-1253,
1178-1200).

## 22.5 Template Body

```
TemplateBodyItem ::= FieldDecl | Constructor | StaticMethod

FieldDecl  ::= Identifier ["<" Annotation ">"] [":" TypeExpr] ["=" DefaultValue]
Constructor ::= "$function" "." "@" ["<" ParamList ">"] ":" ( "(" Body ")" | "$default" )
StaticMethod ::= "$function" "." ["<" GenericParams ">"] ["!"] "#" MethodName
                 ["<" GenericArgs ">"] "(" ParamList ")" ":" ReturnType "(" Body ")"
```

In-template static methods accept the same signature sugar as standalone
functions (`Signature`, 22.6): `: (Params) -> Ret (Body)`,
`<Params> : Ret (Body)`, `(Params) : Ret (Body)`, or bodyless `: Ret`
(parser.c:1613-1625, via `parse_function_signature` at 1136-1166).

## 22.6 Parameters and Body

```
ParamList  ::= (Param ("," Param)*)?
Param      ::= Identifier ":" TypeExpr | "_$" digits ":" TypeExpr

Signature  ::= [ParamsThen] [":" TypeExpr | "->" TypeExpr]     (sugar, may be empty)
ParamsThen ::= "<" ParamList ">" | ":" "(" ParamList ")" | "(" ParamList ")"
```

Body       ::= (Stmt ("," Stmt)* ","?)? [Expr]
Stmt       ::= Bind | Assign | Return | ExprStmt
Bind       ::= Identifier ":" TypeExpr "=" Expr
Assign     ::= "$this" "." Identifier "=" Expr
Return     ::= "$return" "<" TypeExpr ">" "(" Expr ")"
```

## 22.7 Type Expressions

```
TypeExpr   ::= ( Primitive | ArrayType | NamedType | DynType
               | MapType | InlineObject ) ["?"]

Primitive  ::= "String" | "Integer" | "Float" | "Boolean" | "None"
ArrayType  ::= "Array" ["<" TypeExpr ">"]
MapType    ::= "Map"                         (open map value, e.g. "Map?")
NamedType  ::= Name                          (dotted $typedef / $enum / $temp)
DynType    ::= "dyn" Identifier
InlineObject ::= "(" FieldType ("," FieldType)* ","? ")"
FieldType  ::= Identifier ":" TypeExpr ["=" DefaultValue]
```

Additions:
- **`"?"` nullable suffix** — postfix on any type (parser.c:890-893):
  `Url?`, `Map?`, `(type: ProxyType, host: String, port: Integer)?`.
- **`Map` primitive** — open map type (parser.c:819), distinct from
  `( FieldList )` inline objects.
- **`Array`** — currently emitted by the parser as bare `Array` with a `None`
  element type (parser.c:820-823).
- **dotted `Name`** — named types reference dotted templates: `Content.List?`,
  `Agent.Topology`, `Message.List` (parser.c:824-829).
- **inline object `( FieldList )`** — an anonymous structural type with
  per-field defaults (parser.c:830-882; `= DefaultValue` at 854-860):
  `timeout: (connect: Integer = 10, transfer: Integer = 60)`.

## 22.8 Expressions

```
Expr       ::= LiteralExpr | Identifier | "$this"
             | "(" Expr ")"               (compound $)
             | OpExpr
             | CallExpr

OpExpr     ::= Expr BinaryOp Expr
             | UnaryOp Expr
             | Expr "|" Expr              (coalesce)

BinaryOp   ::= "+" | "-" | "*" | "/"
             | "==" | "!=" | "<" | ">" | "<=" | ">="
             | "&&" | "||"

UnaryOp    ::= "!"

CallExpr   ::= "$format" "(" Expr ")"
             | "#" Identifier "(" Args ")"                           (instantiate)
             | "#" Identifier "(." Field ":" Value ("," Field ":")* ")"  (named inst)
             | "#" Identifier ":" Identifier "(" Args ")"           (static call via #)
             | Identifier "." Identifier "(" Args ")"               (static call via dot)
             | "#" Name ":" "(" ( FieldList | Args ) ")"            (colon instantiation)
             | Name "." Identifier "(" Args ")"                     (static call via dot, dotted tname)
             | "$" Identifier "(" Args ")"                          (value-layer function call)
```

New forms (todo 5 parser, parser.c):
- **Colon-instantiation `#Name: ( ... )`** — named (`#Gateway: (servers: (...), ...)`)
  or positional (`#Tool.Function: ("shell", "Execute shell command", "{}")`);
  dotted `Name` allowed; named fields may carry a leading `.` sugar
  (parser.c:521-588 named, 590-599 positional).
- **Static call via `#`** with dotted name: `#T:method(args)` — parser.c:602-647.
- **Static call via dot** with dotted template name: `A.B.method(args)` and the
  plain `T.method(args)` form — parser.c:727-776.
- **Value-layer function call `$name(args)`** — resolves a `$function.#name`
  declaration or the builtin `env` (parser.c:713-725; `env` eval at
  expr.c:1092-1103: `$env("VAR")` returns the variable value, or `None` if unset).

## 22.9 Comments and Whitespace

```
Comment ::= "/*" (any char)* "*/"
Whitespace ::= space | tab | newline | carriage return
```

## 22.10 Lexical Elements

```
Identifier   ::= (letter | "_") (letter | digit | "_")*
Keywords     ::= "true" | "false" | "None"
               | "String" | "Integer" | "Float" | "Boolean"
               | "Array" | "Map" | "dyn" | "this"
$-Keywords   ::= "$" letter+
                 Recognized: "$typedef" "$enum" "$temp" "$function"
                             "$default" "$this" "$return" "$format"
                 Any other "$name" (e.g. "$env") lexes as a "$"-prefixed
                 identifier and is used as a value-layer function call (22.8).
```

`?` is a single delimiter token (`CCO_TK_QMARK`) used as the nullable type
suffix (22.7); `.` is the dotted-name separator (22.4, 22.7, 22.8).

## 22.11 QA Checklist — new syntax vs parser.c

| Syntax | EBNF addition | parser.c line | search-file evidence |
|--------|---------------|---------------|----------------------|
| `?` nullable type suffix | 22.7 `TypeExpr ... ["?"]` | 890-893 | `system.cco:24` `region: String?`; `system.cco:31` `key: String?` |
| `Map` type keyword | 22.7 `MapType ::= "Map"` | 819 | `system.cco:45` `meta: Map?` |
| `Array` type | 22.7 `ArrayType` (existing) | 820-823 | `system.cco:67` `servers: Array`; `system.cco:35` `status_codes: Array = (429, 502, 503)` |
| dotted `Name` in `TypeExpr` | 22.7 `NamedType ::= Name` | 824-829 | `system.cco:44` `provider: Provider.Vendor?`; `system.cco:79` `tuning: Tuning.Params?` |
| inline object type `( FieldList )` + per-field defaults | 22.7 `InlineObject` / `FieldType` | 830-882 (defaults 854-860) | `system.cco:32` `timeout: (connect: Integer = 10, transfer: Integer = 60)`; `system.cco:33-35` nested `retry`/`backoff` |
| dotted template decl names | 22.4 `Template ::= ... Name ...` | 1526 | `system.cco:22` `$temp.Provider.Vendor:`; `system.cco:52` `$temp.Hook.Function:` |
| dotted template parent `+ Name` | 22.4 `Template ::= ... ["+" ParentName]` | 1534 | `request.cco:14` `$temp.CdnHook + Hook.Function:` |
| `$function.#name: (Params) -> Ret [Body]` | 22.4 `StandaloneFunction` + 22.6 `Signature` | 1256-1290, 1136-1166 | `system.cco:83` `$function.#env: (name: String) -> String` |
| `$function.#name<Params>: Ret (Body)` | 22.4 `StandaloneFunction` + 22.6 `Signature` | 1139, 1141-1148, 1157 | learn/20 `$function.#square<x: Integer>: Integer (...)` |
| top-level `$function.@<Params>: (Body)` constructor attach | 22.4 `AttachedConstructor` | 1221-1253 (flush 1178-1200) | learn/19 `$function.@<host: String, port: Integer>: (...)` |
| in-template static methods | 22.5 `StaticMethod` (existing) + `Signature` sugar | 1582-1630, 1613-1625 | learn/20 `$function.#square<x: Integer>: Integer (...)` |
| static call `#T:method(args)` | 22.8 `CallExpr` `# Name ":" Identifier "(" Args ")"` | 602-647 | learn/20 `#Math:square(5)` |
| static call `T.method(args)` | 22.8 `CallExpr` dot forms | 727-776 | learn/20 `Math:square(6)` |
| colon-instantiation `#Name: ( ... )` | 22.8 `CallExpr` `# Name ":" "(" ... ")"` | 521-588 (named), 590-599 (positional) | `request.cco:18` `gateway: #LoadBalancer: (`; `request.cco:72` `#CdnHook: ("purge", ...)` |
| value-layer `$name(args)` call | 22.2 `FunctionCall` + 22.8 | 713-725 | `request.cco:25` `key: $env("CLUSTER_TOKEN")` |
| `env` builtin | 22.2 `FunctionCall` note | expr.c 1092-1103 | `request.cco:25,39` `$env(...)` set/unset → value/None |
| root-less files | 22.1 parsing note | 1833-1861 (`parse_merged_text`), 1917 (`cco_parse_full_froms`) | `system.cco` (schema-only, parsed via from_files) |
