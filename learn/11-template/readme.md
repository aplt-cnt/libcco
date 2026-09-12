# Chapter 11: What is a Template?

After this chapter you will understand what CCO "templates" are and what problem they solve.

## 11.1 The Repetition Problem

Suppose you have many servers to configure:

```cco
server1: (
    host: "web1.example.com",
    port: 8080
),
server2: (
    host: "web2.example.com",
    port: 8080
),
server3: (
    host: "web3.example.com",
    port: 8080
)
```

Now you need to change `port` to 9090 -- you have to change it in three places. What if there were 30 servers?

## 11.2 Template = Blueprint

A template (`$temp`) acts as a **blueprint** defining what fields a "server" has. When you need a server, you instantiate the blueprint with specific values:

```cco
$temp.Server: (
    host: String,
    port: Integer
),

server1: #Server("web1.example.com", 8080),
server2: #Server("web2.example.com", 9090)
```

- `$temp.Server` defines a template called `Server`
- `#Server(...)` means "create an instance from this template"
- To change the port, you only need to change `$temp.Server`'s definition

## 11.3 Comparison with Programming

| Concept | Analogy |
|---------|---------|
| Template `$temp` | class, struct |
| Instance `#Server(...)` | object, instance |
| Field `host: String` | property, member variable |

## 11.4 Summary

- Templates solve the "repeating the same structure" problem
- `$temp.Name: (...)` defines a template
- `#Name(...)` creates an instance of a template

Next chapter -- template fields and default values.
