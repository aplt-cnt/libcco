# Chapter 19: Constructors `$function.@`

After this chapter you will be able to define custom constructors with initialization logic.

## 19.1 Why Constructors?

Currently, instantiation just assigns values to fields:

```cco
$temp.Point: (x: Integer, y: Integer),
origin: #Point(0, 0)
```

But what if initialization is more complex -- computing one field from another, or validation?

## 19.2 The Default Constructor

If no `$function.@` is defined, CCO auto-generates a **default constructor** that assigns positional args to fields in order. All previous `#Server(...)` examples used the default constructor.

Explicitly:

```cco
$temp.Server: (
    host: String,
    port: Integer = 8080
),
/* Same as the auto-generated default */
$function.@: $default
```

## 19.3 Custom Constructor

```cco
$temp.Server: (
    host: String,
    port: Integer = 8080,
    url: String = ""
),

$function.@<host: String, port: Integer>: (
    $this.(host: host, port: port, url: ("https://" + host + ":" + port))
)
```

Breaking it down:

- `$function.@<host: String, port: Integer>: (...)` 
  - `$function.@` means "I am defining a constructor"
  - `<host: String, port: Integer>` is the parameter list
  - `:` then `(...)` is the method body

Inside the body, `$this.(...)` assigns to the instance's fields:

```cco
$this.(host: host, port: port, url: ...)
```

This means: set field `host` to parameter `host`, field `port` to parameter `port`, field `url` to the computed expression.

## 19.4 Parameter Names vs Field Names

Constructor parameters can have the **same name** as fields (most common):

```cco
$function.@<host: String, port: Integer>: (
    $this.(host: host, port: port)
    /* NOTE:       ^field   ^param */
)
```

Or different names:

```cco
$function.@<h: String, p: Integer>: (
    $this.(host: h, port: p)
)
```

## 19.5 Anonymous Parameters `_$0`, `_$1`

If parameter names are unimportant:

```cco
$function.@<_$0: String, _$1: Integer>: (
    $this.(host: _$0, port: _$1)
)
```

## 19.6 Multiple Constructors

A template can have multiple constructors. If you define at least one custom constructor, the default is **not** auto-generated.

```cco
$temp.Server: (
    host: String,
    port: Integer = 8080
),

/* Custom constructor: only accepts host */
$function.@<host: String>: (
    $this.(host: host, port: 8080)
)

/* NOTE: only the custom constructor is available */
s1: #Server("localhost")     /* OK */
/* s2: #Server("local", 443) /* ERROR -- no matching constructor */
```

## 19.7 Summary

- `$function.@<params>: (body)` defines a custom constructor
- Body uses `$this.(...)` to assign fields
- Custom constructors replace the default

Next chapter -- static methods.
