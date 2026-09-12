# Chapter 1: What is CCO?

After this chapter you will know:
- What CCO is
- How it differs from JSON, INI, YAML
- What a complete `.cco` file looks like

## 1.1 In One Sentence

**CCO** = **C**NT **C**onfiguration **O**bject.

It is a **human-readable configuration file format**, in the same category as JSON, YAML, and INI. But CCO does a few things they cannot:

- Define **type aliases** (e.g. `$typedef.Url: String`)
- Define **enumerations** (e.g. `$enum.Mode = (DEV, STAGING, PROD)`)
- Define **templates** (like "classes" or "structs" in programming languages)
- Write **expressions** inside configs (e.g. `$(10 + 20 * 3)`)

## 1.2 A Quick Preview

```cco
/* A web server configuration */

$typedef.Url: String,
$enum.Mode = (DEV, STAGING, PROD),

$temp.Server: (
    host: String,
    port: Integer = 8080,
    mode: Mode = DEV
),

app: (
    name: "libcco-demo",
    version: 0.1,
    servers: (
        #Server("localhost"),
        #Server("api.example.com", 443, PROD)
    )
)
```

Do not worry if none of this makes sense yet -- after the tutorial it will feel simpler than JSON.

## 1.3 CCO vs Other Formats

| Need | JSON | YAML | CCO |
|------|------|------|-----|
| Comments | no | `#` supported | `/* */` supported |
| Type aliases | no | no | `$typedef` |
| Enumerations | no | no | `$enum` |
| Reusable structures | no | no | `$temp` |
| Expressions | no | no | `$(...)` |
| Learning curve | low | medium | low-medium |
| Expressiveness | weak | moderate | strong |

## 1.4 What You Will Be Able to Do

1. Read any `.cco` configuration file
2. Write well-structured `.cco` files yourself
3. Use the libcco C library to parse and generate `.cco` files (that is a separate document)

Next chapter -- your first `.cco` file.
