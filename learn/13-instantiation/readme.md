# Chapter 13: Instantiation -- Positional Arguments

After this chapter you will be able to create template instances by passing positional arguments.

## 13.1 What is Instantiation?

After defining a template, you use `#TemplateName(...)` to create an **instance** of it.

## 13.2 Positional Arguments

The simplest way: pass values in field order.

```cco
$temp.Point: (x: Integer, y: Integer),

origin: #Point(0, 0),         /* NOTE: x=0, y=0 */
p1:     #Point(3, 5)          /* NOTE: x=3, y=5 */
```

`#Point(0, 0)` -- first value `0` goes to first field `x`, second value `0` to second field `y`. This is **positional passing**.

## 13.3 With Default Values

```cco
$temp.Server: (
    host: String,
    port: Integer = 8080,
    timeout: Integer = 30
),

s1: #Server("localhost"),           /* NOTE: only host; port and timeout use defaults */
s2: #Server("example.com", 443),    /* NOTE: specify host and port; timeout default */
s3: #Server("test.com", 80, 60)     /* NOTE: all specified */
```

Fields with defaults can be omitted, as long as all preceding required fields are supplied.

## 13.4 Nested Instantiation

```cco
$temp.Endpoint: (
    name: String,
    url: String
),

$temp.App: (
    endpoints: Array
),

app: #App((
    #Endpoint("api", "https://api.example.com"),
    #Endpoint("admin", "https://admin.example.com")
))
```

## 13.5 Summary

- `#TemplateName(val1, val2, ...)` passes args positionally
- Fields with defaults can be omitted
- Instances can nest

Next chapter -- named instantiation (passing args by name).
