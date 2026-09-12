# Chapter 2: Your First .cco File

After this chapter you will be able to:
- Write the simplest `.cco` file
- Understand the basic structure of a `.cco` file

## 2.1 The Shortest Config File

Open an editor, create `hello.cco`, and type:

```cco
name: "world"
```

Save it. This is a valid `.cco` file.

## 2.2 What Does It Mean?

- `name` -- a key (a label)
- `:` -- a colon separating the key and value
- `"world"` -- a value (a string in this case)

The line means: **"set key name to value world"**.

## 2.3 Multiple Keys

```cco
name: "libcco",
version: 1
```

There are two key-value pairs, separated by a **comma `,`** .

## 2.4 Important Rules

- A key and value must be separated by a colon `:`
- Multiple key-value pairs must be separated by commas `,`
- Strings must be wrapped in double quotes `"..."`

## 2.5 Common Mistakes

```cco
/* NOTE: missing colon -- ERROR */
name "world"

/* NOTE: missing comma -- ERROR */
name: "libcco"
version: 1

/* NOTE: unquoted string -- ERROR */
name: world
```

```cco
/* NOTE: correct */
name: "libcco",
version: 1
```

## 2.6 Summary

- The basic unit in CCO is a `key: value` pair
- Multiple pairs are separated by commas
- Strings must be wrapped in `"..."`

Next chapter -- basic data types.
