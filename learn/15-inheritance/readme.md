# Chapter 15: Template Inheritance

After this chapter you will be able to extend existing templates using inheritance.

## 15.1 The Problem

You have a basic server template:

```cco
$temp.Server: (
    host: String,
    port: Integer = 8080
)
```

Now you need a "secure server" that has everything `Server` has, plus a `cert` and `ssl_port`.

## 15.2 Inheritance with `+ Parent`

```cco
$temp.SecureServer + Server: (
    cert: String,
    ssl_port: Integer = 443
)
```

Syntax: `$temp.ChildName + ParentName: ( ... )`

`SecureServer` automatically has all of `Server`'s fields (`host`, `port`), plus its own new fields (`cert`, `ssl_port`).

## 15.3 Using an Inherited Template

```cco
/* NOTE: positional args -- parent fields first, child fields after */
s1: #SecureServer("localhost", 8080, "/etc/cert.pem", 443),

/* NOTE: named args (recommended) */
s2: #SecureServer(.host: "api.example.com", .cert: "/etc/cert.pem", .ssl_port: 8443)
```

With positional args, **parent fields come first**, then child fields.

## 15.4 Multiple Inheritance?

CCO supports **single inheritance only** -- one parent per template. But chains can be long:

```cco
$temp.Shape: ( area: Float ),
$temp.Circle + Shape: ( radius: Float ),
$temp.SmallCircle + Circle: ( label: String ),

/* SmallCircle has area + radius + label */
c: #SmallCircle(3.14, 1.5, "tiny")
```

## 15.5 Common Use Cases

1. **Base + extension**: `Server` -> `SecureServer`
2. **Abstract + concrete**: `Animal` -> `Dog`, `Cat`
3. **Layered configs**: `BaseConfig` -> `DatabaseConfig`, `CacheConfig`

```cco
$temp.BaseEndpoint: (
    url: String,
    timeout: Integer = 30
),

$temp.RESTEndpoint + BaseEndpoint: (
    method: String = "GET"
),

$temp.GraphQLEndpoint + BaseEndpoint: (
    schema: String
)
```

## 15.6 Summary

- `$temp.Child + Parent: (...)` creates an inherited template
- Child has all parent fields
- Single inheritance only (but chains can be long)
- Named instantiation is especially useful with inheritance

Congratulations! You have learned CCO's core features. Next chapter -- expressions.
