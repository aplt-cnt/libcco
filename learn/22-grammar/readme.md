# Chapter 22: Complete Grammar Reference

This is the full CCO grammar specification in EBNF-like notation. Useful for quick reference.

## 22.1 File Structure

```
File       ::= (Declaration ",")* RootValue ","?
RootValue  ::= Map | Array | Literal | TemplateInstance
```

## 22.2 Values

```
Value      ::= Map | Array | Literal
             | TemplateInstance | StaticCall
             | EnumConstant | Identifier

Map        ::= "(" Field ("," Field)* ","? ")"
             | Identifier ":" Value           (top-level shorthand, file root only)
Field      ::= Identifier ":" Value

Array      ::= "(" Value ("," Value)* ","? ")"
Empty "()" ::= treated as empty Map
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
Declaration ::= TypeAlias | Enum | Template

TypeAlias  ::= "$typedef" "." Identifier ":" TypeExpr
Enum       ::= "$enum" "." Identifier "=" "(" Identifier ("," Identifier)* ","? ")"
Template   ::= "$temp" ["<" GenericParam ">"] "." Identifier ["+" ParentName] ":" "(" TemplateBodyItem* ")"
```

## 22.5 Template Body

```
TemplateBodyItem ::= FieldDecl | Constructor | StaticMethod

FieldDecl  ::= Identifier ["<" Annotation ">"] [":" TypeExpr] ["=" DefaultValue]
Constructor ::= "$function" "." "@" ["<" ParamList ">"] ":" ( "(" Body ")" | "$default" )
StaticMethod ::= "$function" "." ["<" GenericParams ">"] ["!"] "#" MethodName
                 ["<" GenericArgs ">"] "(" ParamList ")" ":" ReturnType "(" Body ")"
```

## 22.6 Parameters and Body

```
ParamList  ::= (Param ("," Param)*)?
Param      ::= Identifier ":" TypeExpr | "_$" digits ":" TypeExpr

Body       ::= (Stmt ("," Stmt)* ","?)? [Expr]
Stmt       ::= Bind | Assign | Return | ExprStmt
Bind       ::= Identifier ":" TypeExpr "=" Expr
Assign     ::= "$this" "." Identifier "=" Expr
Return     ::= "$return" "<" TypeExpr ">" "(" Expr ")"
```

## 22.7 Type Expressions

```
TypeExpr   ::= Primitive | ArrayType | NamedType | DynType
Primitive  ::= "String" | "Integer" | "Float" | "Boolean" | "None"
ArrayType  ::= "Array" ["<" TypeExpr ">"]
NamedType  ::= Identifier            (references $typedef / $enum / $temp)
DynType    ::= "dyn" Identifier
```

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
```

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
               | "Array" | "dyn" | "this"
$-Keywords   ::= "$" letter+
                 Recognized: "$typedef" "$enum" "$temp" "$function"
                             "$default" "$this" "$return" "$format"
```
