/**
 * @file cco.h
 * @brief libcco – C library for parsing, manipulating, and serializing .cco configuration files.
 *
 * This header implements the complete CCO language specification version 0.1.0,
 * including type aliases, enumerations, templates (classes with inheritance),
 * constructors, static methods, generics, dynamic dispatch, expressions,
 * statements, and a full declaration system.
 *
 * @section usage Example
 * @code
 * #include <cnt/cco.h>
 * #include <stdio.h>
 *
 * int main() {
 *     const char *src =
 *         "$typedef.Url: String,\n"
 *         "$enum.HttpMethod = (GET, POST),\n"
 *         "$temp.Endpoint: ( url<Url>: Url, method: HttpMethod ),\n"
 *         "api: #Endpoint(\"https://api.example.com\", GET)\n";
 *
 *     cco_parse_result_t *res = cco_parse_full(src);
 *     if (!res) {
 *         fprintf(stderr, "Parse error: %s\n", cco_last_error());
 *         return 1;
 *     }
 *
 *     cco_object_t *root = cco_parse_result_root(res);
 *     cco_object_t *api = cco_object_get(root, "api");
 *     const char *url = cco_string_get(cco_object_get(api, "url"));
 *     printf("API URL: %s\n", url);
 *
 *     cco_parse_result_free(res);
 *     return 0;
 * }
 * @endcode
 */

#ifndef CCO_H
#define CCO_H

#include <stddef.h>   // size_t
#include <stdarg.h>   // va_list (for varargs constructors)
#include <stdio.h>    // FILE

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Version
 *============================================================================*/

/**
 * @def CCO_VERSION_MAJOR
 * @brief Major version number (0 for development).
 */
#define CCO_VERSION_MAJOR 0

/**
 * @def CCO_VERSION_MINOR
 * @brief Minor version number.
 */
#define CCO_VERSION_MINOR 1

/**
 * @def CCO_VERSION_PATCH
 * @brief Patch version number.
 */
#define CCO_VERSION_PATCH 0

/*==============================================================================
 * Basic Types and Enumerations
 *============================================================================*/

/**
 * @brief Runtime value type tags for .cco objects.
 *
 * These are used to identify the kind of data stored in a #cco_object_t.
 */
typedef enum cco_value_type {
    CCO_TYPE_NONE,              /**< None value (null) */
    CCO_TYPE_STRING,            /**< UTF-8 string */
    CCO_TYPE_INT,               /**< Signed 64-bit integer */
    CCO_TYPE_FLOAT,             /**< Double-precision floating point */
    CCO_TYPE_BOOL,              /**< Boolean (true/false) */
    CCO_TYPE_TEMPLATE_INSTANCE, /**< Instance of a template (carries its template name) */
    CCO_TYPE_MAP,               /**< Key-value map (dictionary) */
    CCO_TYPE_ARRAY              /**< Ordered list of values */
} cco_value_type_t;

/**
 * @brief Kinds of type expressions in the type system.
 */
typedef enum cco_type_kind {
    CCO_TYPEKIND_PRIMITIVE,   /**< Primitive type (String, Integer, etc.) */
    CCO_TYPEKIND_ARRAY,       /**< Array<T> */
    CCO_TYPEKIND_OBJECT,      /**< Object type (field: Type, ...) */
    CCO_TYPEKIND_NAMED,       /**< Named type (alias, enum, template name) */
    CCO_TYPEKIND_DYN          /**< dyn BaseTemplate (dynamic dispatch) */
} cco_type_kind_t;

/**
 * @brief Primitive type specifiers for #cco_type_primitive().
 */
typedef enum cco_primitive_type {
    CCO_PRIM_STRING,   /**< String type */
    CCO_PRIM_INTEGER,  /**< Integer type */
    CCO_PRIM_FLOAT,    /**< Float type */
    CCO_PRIM_BOOLEAN,  /**< Boolean type */
    CCO_PRIM_NONE      /**< None type */
} cco_primitive_type_t;

/**
 * @brief Operators for expression AST nodes.
 */
typedef enum cco_op {
    CCO_OP_ADD,      /**< Addition (+) */
    CCO_OP_SUB,      /**< Subtraction (-) */
    CCO_OP_MUL,      /**< Multiplication (*) */
    CCO_OP_DIV,      /**< Division (/) */
    CCO_OP_EQ,       /**< Equality (==) */
    CCO_OP_NE,       /**< Inequality (!=) */
    CCO_OP_LT,       /**< Less than (<) */
    CCO_OP_GT,       /**< Greater than (>) */
    CCO_OP_LE,       /**< Less than or equal (<=) */
    CCO_OP_GE,       /**< Greater than or equal (>=) */
    CCO_OP_AND,      /**< Logical AND (&&) */
    CCO_OP_OR,       /**< Logical OR (||) */
    CCO_OP_NOT,      /**< Logical NOT (!) – unary */
    CCO_OP_COALESCE  /**< Coalescing (|) – returns left if not None, else right */
} cco_op_t;

/*==============================================================================
 * Opaque Handles
 *============================================================================*/

/**
 * @brief Handle to any runtime .cco value (map, array, string, number, bool, None, template instance).
 *
 * All values are reference-counted or owned by their container. Do not free a value
 * that is part of a larger tree; instead free the root.
 */
typedef struct cco_object cco_object_t;

/**
 * @brief Handle to a standalone .cco array.
 *
 * Arrays are created with cco_array_new() and can be wrapped into a #cco_object_t
 * using cco_array_wrap().
 */
typedef struct cco_array cco_array_t;

/**
 * @brief Symbol table holding all type aliases, enumerations, and template definitions.
 */
typedef struct cco_symbol_table cco_symbol_table_t;

/**
 * @brief Type expression tree (for type annotations and generic constraints).
 */
typedef struct cco_type_expr cco_type_expr_t;

/**
 * @brief Template definition (class) with fields, constructors, and static methods.
 */
typedef struct cco_template cco_template_t;

/**
 * @brief Constructor definition (instance constructor) belonging to a template.
 */
typedef struct cco_constructor cco_constructor_t;

/**
 * @brief Static method definition belonging to a template.
 */
typedef struct cco_static_method cco_static_method_t;

/**
 * @brief Expression AST node.
 */
typedef struct cco_expr cco_expr_t;

/**
 * @brief Statement AST node (used in method bodies).
 */
typedef struct cco_stmt cco_stmt_t;

/**
 * @brief List of parameters for a constructor or static method.
 */
typedef struct cco_param_list cco_param_list_t;

/**
 * @brief List of field initializers for named instantiation ( .field: value ).
 */
typedef struct cco_field_init_list cco_field_init_list_t;

/**
 * @brief Container for a method body (sequence of statements followed by an optional final expression).
 */
typedef struct cco_method_body cco_method_body_t;

/**
 * @brief Result of a full parse (symbol table + root value).
 */
typedef struct cco_parse_result cco_parse_result_t;

/**
 * @brief Iterator for traversing key-value pairs in a map.
 */
typedef struct cco_map_iter cco_map_iter_t;

/*==============================================================================
 * Value Layer (Objects, Arrays, Primitives)
 *============================================================================*/

/** @name Value construction and destruction */
///@{

/**
 * @brief Create a new empty map object.
 * @return Newly allocated map, or NULL on memory allocation failure.
 * @note The map must be freed with cco_object_free().
 */
cco_object_t* cco_object_new(void);

/**
 * @brief Create a None value object.
 * @return A cco_object_t representing None, or NULL on allocation failure.
 */
cco_object_t* cco_none_new(void);

/**
 * @brief Create a string object.
 * @param s Null-terminated UTF-8 string. Must not be NULL.
 * @return New string object, or NULL on allocation failure.
 * @note The string content is copied internally.
 */
cco_object_t* cco_string_new(const char *s);

/**
 * @brief Create an integer object.
 * @param i Integer value (64-bit signed).
 * @return New integer object, or NULL on allocation failure.
 */
cco_object_t* cco_int_new(long long i);

/**
 * @brief Create a floating-point object.
 * @param f Double value.
 * @return New float object, or NULL on allocation failure.
 */
cco_object_t* cco_float_new(double f);

/**
 * @brief Create a boolean object.
 * @param val 0 for false, non-zero for true.
 * @return New boolean object, or NULL on allocation failure.
 */
cco_object_t* cco_bool_new(int val);

/**
 * @brief Wrap an existing array into a cco_object_t.
 * @param arr Array to wrap (must not be NULL). Ownership is transferred.
 * @return cco_object_t representing the array, or NULL on failure.
 * @warning After wrapping, do not use the original array handle; the object owns it.
 */
cco_object_t* cco_array_wrap(cco_array_t *arr);

/**
 * @brief Destroy an object and recursively free all its children.
 * @param obj Object to free. May be NULL (does nothing).
 * @note Arrays wrapped inside an object are freed recursively.
 */
void cco_object_free(cco_object_t *obj);

/**
 * @brief Create a deep copy of an object.
 * @param obj Object to copy.
 * @return New object identical to the original, or NULL on allocation failure.
 */
cco_object_t* cco_object_copy(const cco_object_t *obj);

///@}

/**
 * @brief Create a template instance object.
 * @param template_name Name of the template that this instance instantiates.
 * @return New template instance object (behaves like a map with hidden type info), or NULL on error.
 * @note This is used internally by the instantiation machinery; user code typically does not call it directly.
 */
cco_object_t* cco_template_instance_new(const char *template_name);

/**
 * @brief Get the concrete template name of a template instance.
 * @param obj Template instance object (must be CCO_TYPE_TEMPLATE_INSTANCE).
 * @return Template name, or NULL if @p obj is not a template instance.
 */
const char* cco_object_template_name(const cco_object_t *obj);

/** @name Array operations (standalone arrays) */
///@{

/**
 * @brief Create a new empty array.
 * @return New array, or NULL on allocation failure.
 */
cco_array_t* cco_array_new(void);

/**
 * @brief Destroy an array and all its elements.
 * @param arr Array to free. May be NULL.
 */
void cco_array_free(cco_array_t *arr);

/**
 * @brief Create a deep copy of an array.
 * @param arr Array to copy.
 * @return New array identical to the original, or NULL on failure.
 */
cco_array_t* cco_array_copy(const cco_array_t *arr);

/**
 * @brief Append an element to the end of an array.
 * @param arr Array object.
 * @param value Value to append (ownership transferred).
 * @return 0 on success, -1 on failure.
 */
int cco_array_add(cco_array_t *arr, cco_object_t *value);

/**
 * @brief Insert an element at a given index.
 * @param arr Array object.
 * @param index Position to insert (0 <= index <= size).
 * @param value Value to insert (ownership transferred).
 * @return 0 on success, -1 on failure (invalid index or allocation error).
 */
int cco_array_insert(cco_array_t *arr, size_t index, cco_object_t *value);

/**
 * @brief Remove an element at a given index.
 * @param arr Array object.
 * @param index Index of element to remove.
 * @return 0 on success, -1 on failure (invalid index or NULL arr).
 */
int cco_array_remove(cco_array_t *arr, size_t index);

/**
 * @brief Retrieve an element from an array by index.
 * @param arr Array object.
 * @param index Index (0 <= index < size).
 * @return Pointer to the element, or NULL if index out of bounds or arr NULL.
 * @warning The returned pointer is owned by the array and must not be freed.
 */
cco_object_t* cco_array_get(const cco_array_t *arr, size_t index);

/**
 * @brief Return the number of elements in an array.
 * @param arr Array object.
 * @return Number of elements (0 if arr is NULL).
 */
size_t cco_array_size(const cco_array_t *arr);

/**
 * @brief Remove all elements from an array.
 * @param arr Array object.
 */
void cco_array_clear(cco_array_t *arr);

///@}

/** @name Map operations (on objects of type MAP) */
///@{

/**
 * @brief Set a key-value pair in a map.
 * @param obj Map object (must be CCO_TYPE_MAP).
 * @param key Null-terminated string key.
 * @param value Value object to insert (ownership transferred).
 * @return 0 on success, -1 on error (obj not a map, key NULL, or allocation failure).
 * @note If the key already exists, the old value is freed and replaced.
 */
int cco_object_set(cco_object_t *obj, const char *key, cco_object_t *value);

/**
 * @brief Delete a key from a map.
 * @param obj Map object.
 * @param key Key to delete.
 * @return 0 on success, -1 if key not found or obj is not a map.
 * @note The associated value is freed automatically.
 */
int cco_object_del(cco_object_t *obj, const char *key);

/**
 * @brief Remove all key-value pairs from a map.
 * @param obj Map object.
 */
void cco_object_clear(cco_object_t *obj);

/**
 * @brief Retrieve a value from a map by key.
 * @param obj Map object.
 * @param key Key to look up.
 * @return Pointer to the value object, or NULL if not found or obj is not a map.
 * @warning The returned pointer is owned by the map and must not be freed.
 */
cco_object_t* cco_object_get(const cco_object_t *obj, const char *key);

/**
 * @brief Check if a map contains a key.
 * @param obj Map object.
 * @param key Key to check.
 * @return 1 if key exists and obj is a map, 0 otherwise.
 */
int cco_object_has_key(const cco_object_t *obj, const char *key);

/**
 * @brief Return the number of key-value pairs in a map.
 * @param obj Map object.
 * @return Number of entries (0 if obj is not a map or NULL).
 */
size_t cco_object_size(const cco_object_t *obj);

///@}

/** @name Map iteration */
///@{

/**
 * @brief Create an iterator for a map.
 * @param map Map object (must be CCO_TYPE_MAP).
 * @return New iterator, or NULL on error.
 * @warning The iterator becomes invalid if the map is modified while iterating.
 */
cco_map_iter_t* cco_map_iter_new(const cco_object_t *map);

/**
 * @brief Free a map iterator.
 * @param iter Iterator to free. May be NULL.
 */
void cco_map_iter_free(cco_map_iter_t *iter);

/**
 * @brief Advance the iterator to the next key-value pair.
 * @param iter Iterator.
 * @param key Output parameter for the key (string pointer). May be NULL.
 * @param value Output parameter for the value object. May be NULL.
 * @return 1 (true) if a pair was retrieved, 0 (false) at end of map.
 * @note The key pointer is valid only until the map is modified or freed.
 *       The value is owned by the map.
 */
int cco_map_iter_next(cco_map_iter_t *iter, const char **key, cco_object_t **value);

///@}

/** @name Type queries and accessors */
///@{

/**
 * @brief Get the runtime type of a .cco object.
 * @param obj Object to query.
 * @return Type of the object, or CCO_TYPE_NONE if obj is NULL.
 */
cco_value_type_t cco_object_type(const cco_object_t *obj);

/**
 * @brief Extract a string from a string object.
 * @param obj Object (expected CCO_TYPE_STRING).
 * @return Null-terminated string, or "" on type mismatch or NULL.
 * @warning The returned pointer is owned by the object and must not be freed.
 */
const char* cco_string_get(const cco_object_t *obj);

/**
 * @brief Extract an integer from an integer object.
 * @param obj Object (expected CCO_TYPE_INT).
 * @return Integer value, or 0 on type mismatch or NULL.
 */
long long cco_int_get(const cco_object_t *obj);

/**
 * @brief Extract a double from a float object.
 * @param obj Object (expected CCO_TYPE_FLOAT).
 * @return Double value, or 0.0 on type mismatch or NULL.
 */
double cco_float_get(const cco_object_t *obj);

/**
 * @brief Extract a boolean from a boolean object.
 * @param obj Object (expected CCO_TYPE_BOOL).
 * @return 1 if true, 0 if false (or on type mismatch or NULL).
 */
int cco_bool_get(const cco_object_t *obj);

/**
 * @brief Test whether an object represents None.
 * @param obj Object.
 * @return 1 if obj is CCO_TYPE_NONE, 0 otherwise.
 */
int cco_is_none(const cco_object_t *obj);

/**
 * @brief Retrieve the underlying array from an object that holds an array.
 * @param obj Object (expected CCO_TYPE_ARRAY).
 * @return Pointer to the internal array, or NULL if obj is not an array.
 * @warning The returned array handle is owned by the object and must not be freed.
 */
const cco_array_t* cco_object_as_array(const cco_object_t *obj);

/**
 * @def cco_is_map(obj)
 * @brief Test if an object is a map.
 * @param obj Object to test.
 * @return Non-zero if map, zero otherwise.
 */
#define cco_is_map(obj)   (cco_object_type(obj) == CCO_TYPE_MAP)

/**
 * @def cco_is_array(obj)
 * @brief Test if an object is an array.
 */
#define cco_is_array(obj) (cco_object_type(obj) == CCO_TYPE_ARRAY)

/**
 * @def cco_is_string(obj)
 * @brief Test if an object is a string.
 */
#define cco_is_string(obj)(cco_object_type(obj) == CCO_TYPE_STRING)

/**
 * @def cco_is_int(obj)
 * @brief Test if an object is an integer.
 */
#define cco_is_int(obj)   (cco_object_type(obj) == CCO_TYPE_INT)

/**
 * @def cco_is_float(obj)
 * @brief Test if an object is a float.
 */
#define cco_is_float(obj) (cco_object_type(obj) == CCO_TYPE_FLOAT)

/**
 * @def cco_is_bool(obj)
 * @brief Test if an object is a boolean.
 */
#define cco_is_bool(obj)  (cco_object_type(obj) == CCO_TYPE_BOOL)

/**
 * @def cco_is_template_instance(obj)
 * @brief Test if an object is a template instance.
 */
#define cco_is_template_instance(obj) (cco_object_type(obj) == CCO_TYPE_TEMPLATE_INSTANCE)

///@}

/*==============================================================================
 * Type System (Type Expressions)
 *============================================================================*/

/**
 * @brief Get the kind of a type expression.
 * @param te Type expression.
 * @return Type kind, or CCO_TYPEKIND_PRIMITIVE if NULL.
 */
cco_type_kind_t cco_type_expr_kind(const cco_type_expr_t *te);

/**
 * @brief Get the primitive type from a primitive type expression.
 * @param te Type expression (must be CCO_TYPEKIND_PRIMITIVE).
 * @return Primitive type, or CCO_PRIM_NONE if NULL/wrong kind.
 */
cco_primitive_type_t cco_type_expr_primitive(const cco_type_expr_t *te);

/**
 * @brief Get the name from a named or dyn type expression.
 * @param te Type expression (must be CCO_TYPEKIND_NAMED or CCO_TYPEKIND_DYN).
 * @return Name string, or NULL if NULL/wrong kind.
 */
const char* cco_type_expr_name(const cco_type_expr_t *te);

/**
 * @brief Create a primitive type expression.
 * @param prim One of #cco_primitive_type_t values.
 * @return New type expression, or NULL on error.
 */
cco_type_expr_t* cco_type_primitive(cco_primitive_type_t prim);

/**
 * @brief Create an array type expression (Array<ElementType>).
 * @param element_type Type expression for the elements.
 * @return New type expression, or NULL on error.
 */
cco_type_expr_t* cco_type_array(cco_type_expr_t *element_type);

/**
 * @brief Structure for a field name/type pair, used with #cco_type_object().
 */
typedef struct cco_field_type_pair {
    const char *name;           /**< Field name */
    cco_type_expr_t *type;      /**< Type of the field */
} cco_field_type_pair_t;

/**
 * @brief Create an object type expression ((field1: Type1, field2: Type2, ...)).
 * @param fields Array of #cco_field_type_pair_t structures.
 * @param count Number of fields.
 * @return New type expression, or NULL on error.
 */
cco_type_expr_t* cco_type_object(const cco_field_type_pair_t *fields, size_t count);

/**
 * @brief Create a named type reference (to an alias, enum, or template).
 * @param name Name of the type.
 * @return New type expression, or NULL on error.
 */
cco_type_expr_t* cco_type_named(const char *name);

/**
 * @brief Create a dynamic type expression (dyn BaseTemplate).
 * @param base_template_name Name of the base template.
 * @return New type expression, or NULL on error.
 */
cco_type_expr_t* cco_type_dyn(const char *base_template_name);

/**
 * @brief Create a deep copy of a type expression.
 * @param te Type expression to copy (may be NULL).
 * @return New type expression, or NULL on error.
 */
cco_type_expr_t* cco_type_expr_copy(const cco_type_expr_t *te);

/**
 * @brief Free a type expression tree.
 * @param te Type expression to free (may be NULL).
 */
void cco_type_expr_free(cco_type_expr_t *te);

/**
 * @brief Compare two type expressions for structural equality.
 * @param a First type expression.
 * @param b Second type expression.
 * @return 1 if equal, 0 otherwise.
 */
int cco_type_expr_equal(const cco_type_expr_t *a, const cco_type_expr_t *b);

/*==============================================================================
 * Symbol Table (Declarations)
 *============================================================================*/

/**
 * @brief Create an empty symbol table.
 * @return New symbol table, or NULL on allocation failure.
 */
cco_symbol_table_t* cco_symbol_table_new(void);

/**
 * @brief Free a symbol table and all contained declarations.
 * @param st Symbol table to free (may be NULL).
 */
void cco_symbol_table_free(cco_symbol_table_t *st);

/**
 * @brief Add a type alias ($typedef).
 * @param st Symbol table.
 * @param name Alias name.
 * @param type Type expression that the alias expands to.
 * @return 0 on success, -1 on error (e.g., name already exists).
 */
int cco_symbol_table_add_typedef(cco_symbol_table_t *st, const char *name, cco_type_expr_t *type);

/**
 * @brief Add an enumeration ($enum).
 * @param st Symbol table.
 * @param name Enum name.
 * @param values Array of string constant names, terminated by NULL.
 * @return 0 on success, -1 on error.
 * @note The array is copied; the caller may free it after the call.
 */
int cco_symbol_table_add_enum(cco_symbol_table_t *st, const char *name, const char **values);

/**
 * @brief Add a template definition ($temp).
 * @param st Symbol table.
 * @param tmpl Template to add.
 * @return 0 on success, -1 on error.
 */
int cco_symbol_table_add_template(cco_symbol_table_t *st, cco_template_t *tmpl);

/**
 * @brief Look up a type by name in the symbol table.
 * @param st Symbol table.
 * @param name Name to look up.
 * @return Type expression if found, NULL otherwise.
 * @note The returned expression is owned by the symbol table and must not be freed.
 */
cco_type_expr_t* cco_symbol_table_lookup_type(const cco_symbol_table_t *st, const char *name);

/**
 * @brief Look up an enum value.
 * @param st Symbol table.
 * @param enum_name Name of the enum.
 * @param value_name Name of the enumeration constant.
 * @return 1 if the value exists, 0 otherwise.
 */
int cco_symbol_table_has_enum_value(const cco_symbol_table_t *st, const char *enum_name, const char *value_name);

/**
 * @brief Look up a template by name.
 * @param st Symbol table.
 * @param name Template name.
 * @return Template if found, NULL otherwise.
 */
cco_template_t* cco_symbol_table_lookup_template(const cco_symbol_table_t *st, const char *name);

/**
 * @brief Get the number of entries in the symbol table (for iteration).
 */
size_t cco_symtab_get_count(const cco_symbol_table_t *st);

/**
 * @brief Get the name of the i-th entry in declaration order.
 */
const char* cco_symtab_get_name(const cco_symbol_table_t *st, size_t idx);

/**
 * @brief Get a typedef type expression by name.
 */
cco_type_expr_t* cco_symtab_get_typedef(const cco_symbol_table_t *st, const char *name);

/**
 * @brief Get enum values and count by name. Returns 1 if found.
 */
int cco_symtab_get_enum_values(const cco_symbol_table_t *st, const char *name,
                                const char ***out_values, size_t *out_count);

/**
 * @brief Get a template definition by name.
 */
cco_template_t* cco_symtab_get_template(const cco_symbol_table_t *st, const char *name);

/*==============================================================================
 * Templates (Classes)
 *============================================================================*/

/**
 * @brief Create a new template definition.
 * @param name Template name.
 * @param parent_name Name of parent template (NULL if none).
 * @return New template, or NULL on error.
 */
cco_template_t* cco_template_new(const char *name, const char *parent_name);

/**
 * @brief Free a template definition.
 * @param tmpl Template to free (may be NULL).
 */
void cco_template_free(cco_template_t *tmpl);

/**
 * @brief Add a field to a template.
 * @param tmpl Template.
 * @param name Field name.
 * @param type Type expression for the field (may be NULL for untyped).
 * @param default_expr Default value expression (may be NULL).
 * @return 0 on success, -1 on error.
 * @note The @p default_expr is evaluated at instantiation time if no argument supplies a value.
 */
int cco_template_add_field(cco_template_t *tmpl, const char *name, cco_type_expr_t *type, cco_expr_t *default_expr);

/**
 * @brief Add a constructor to a template.
 * @param tmpl Template.
 * @param ctor Constructor definition.
 * @return 0 on success, -1 on error.
 */
int cco_template_add_constructor(cco_template_t *tmpl, cco_constructor_t *ctor);

/**
 * @brief Add a static method to a template.
 * @param tmpl Template.
 * @param method Static method definition.
 * @return 0 on success, -1 on error.
 */
int cco_template_add_static_method(cco_template_t *tmpl, cco_static_method_t *method);

/**
 * @brief Get the parent template (resolved from symbol table).
 * @param tmpl Template.
 * @param st Symbol table used for resolution.
 * @return Parent template, or NULL if none or not found.
 */
cco_template_t* cco_template_get_parent(const cco_template_t *tmpl, const cco_symbol_table_t *st);

/**
 * @brief Get the parent template name (may be NULL).
 */
const char* cco_template_get_parent_name(const cco_template_t *tmpl);

/**
 * @brief Get the number of fields in a template.
 */
size_t cco_template_field_count(const cco_template_t *tmpl);

/**
 * @brief Get the name of the i-th field.
 */
const char* cco_template_field_name(const cco_template_t *tmpl, size_t idx);

/*==============================================================================
 * Parameter Lists and Method Bodies
 *============================================================================*/

/**
 * @brief Create a parameter list (for constructors or methods).
 * @param count Number of parameters.
 * @param names Array of parameter names (can be NULL for anonymous). If non-NULL, each name must be a valid identifier.
 * @param types Array of type expressions (may be NULL for untyped). Length must be @p count.
 * @return New parameter list, or NULL on error.
 * @note The names and types are copied; the caller may free them after the call.
 */
cco_param_list_t* cco_param_list_new(size_t count, const char **names, cco_type_expr_t **types);

/**
 * @brief Free a parameter list.
 * @param params Parameter list to free (may be NULL).
 */
void cco_param_list_free(cco_param_list_t *params);

/**
 * @brief Kinds of statements in a method body.
 */
typedef enum cco_stmt_kind {
    CCO_STMT_BIND,      /**< Local variable binding: var : Type? = expr */
    CCO_STMT_ASSIGN,    /**< Field assignment: this.field = expr */
    CCO_STMT_RETURN,    /**< Return statement: $return <Type?>(expr) */
    CCO_STMT_EXPR       /**< Expression statement (value discarded) */
} cco_stmt_kind_t;

/**
 * @brief Create a local variable binding statement.
 * @param name Variable name.
 * @param type Optional type annotation (may be NULL).
 * @param value Initializer expression.
 * @return New statement, or NULL on error.
 */
cco_stmt_t* cco_stmt_bind(const char *name, cco_type_expr_t *type, cco_expr_t *value);

/**
 * @brief Create a field assignment statement (this.field = expr).
 * @param field_name Name of the field.
 * @param value Expression to assign.
 * @return New statement, or NULL on error.
 */
cco_stmt_t* cco_stmt_assign(const char *field_name, cco_expr_t *value);

/**
 * @brief Create a return statement.
 * @param type Optional return type annotation (may be NULL).
 * @param value Expression to return.
 * @return New statement, or NULL on error.
 */
cco_stmt_t* cco_stmt_return(cco_type_expr_t *type, cco_expr_t *value);

/**
 * @brief Create an expression statement (evaluate for side effects, discard value).
 * @param expr Expression to evaluate.
 * @return New statement, or NULL on error.
 */
cco_stmt_t* cco_stmt_expr(cco_expr_t *expr);

/**
 * @brief Free a statement.
 * @param stmt Statement to free (may be NULL).
 */
void cco_stmt_free(cco_stmt_t *stmt);

/**
 * @brief Create an empty method body container.
 * @return New method body, or NULL on allocation failure.
 */
cco_method_body_t* cco_method_body_new(void);

/**
 * @brief Add a statement to a method body.
 * @param body Method body.
 * @param stmt Statement (ownership transferred).
 * @return 0 on success, -1 on error.
 */
int cco_method_body_add_stmt(cco_method_body_t *body, cco_stmt_t *stmt);

/**
 * @brief Set the final expression of a method body (the last expression whose value becomes the return value).
 * @param body Method body.
 * @param expr Final expression (ownership transferred). May be NULL if there is no final expression.
 */
void cco_method_body_set_final_expr(cco_method_body_t *body, cco_expr_t *expr);

/**
 * @brief Free a method body and all its contents.
 * @param body Method body to free (may be NULL).
 */
void cco_method_body_free(cco_method_body_t *body);

/*==============================================================================
 * Constructors and Static Methods
 *============================================================================*/

/**
 * @brief Create a default constructor (equivalent to $function.@: $default).
 * @return Constructor object, or NULL on error.
 * @note The default constructor takes positional arguments in the order of field declarations.
 */
cco_constructor_t* cco_constructor_default(void);

/**
 * @brief Create an explicit constructor.
 * @param params Parameter list (may be NULL for no parameters).
 * @param body Method body (statements + optional final expression). The body should
 *            initialize the instance (usually using $this.(...)).
 * @return Constructor object, or NULL on error.
 */
cco_constructor_t* cco_constructor_new(cco_param_list_t *params, cco_method_body_t *body);

/**
 * @brief Free a constructor.
 * @param ctor Constructor to free (may be NULL).
 */
void cco_constructor_free(cco_constructor_t *ctor);

/**
 * @brief Generic parameter for static methods.
 */
typedef struct cco_generic_param {
    const char *name;                   /**< Generic type name, e.g., "T" */
    cco_type_expr_t *constraint;        /**< Optional constraint, e.g., dyn Character; may be NULL */
} cco_generic_param_t;

/**
 * @brief Create a static method definition.
 * @param name Method name.
 * @param is_private If non-zero, method is private (not inherited).
 * @param generic_params Array of generic parameters (may be NULL if generic_count == 0).
 * @param generic_count Number of generic parameters.
 * @param params Regular parameter list.
 * @param return_type Return type expression (may be NULL for None).
 * @param body Method body.
 * @return New static method, or NULL on error.
 */
cco_static_method_t* cco_static_method_new(
    const char *name, int is_private,
    const cco_generic_param_t *generic_params, size_t generic_count,
    cco_param_list_t *params, cco_type_expr_t *return_type,
    cco_method_body_t *body
);

/**
 * @brief Free a static method.
 * @param method Static method to free (may be NULL).
 */
void cco_static_method_free(cco_static_method_t *method);

/*==============================================================================
 * Expressions
 *============================================================================*/

/**
 * @brief Create a literal value expression.
 * @param val Value object (ownership transferred).
 * @return New expression, or NULL on error.
 */
cco_expr_t* cco_expr_literal(cco_object_t *val);

/**
 * @brief Create an identifier expression (variable reference).
 * @param name Identifier name.
 * @return New expression, or NULL on error.
 * @note The identifier can be a local variable, a parameter, or a field name.
 */
cco_expr_t* cco_expr_identifier(const char *name);

/**
 * @brief Create a reference to $this (static context) or this (instance context).
 * @return New expression, or NULL on error.
 * @note In a static method, this evaluates to the template type; in an instance method,
 *       it evaluates to the current instance.
 */
cco_expr_t* cco_expr_this(void);

/**
 * @brief Create an object literal expression ( { field: expr, ... } ).
 * @param field_names Array of field names.
 * @param field_exprs Array of expressions for each field.
 * @param count Number of fields.
 * @return New expression, or NULL on error.
 */
cco_expr_t* cco_expr_object(const char **field_names, cco_expr_t **field_exprs, size_t count);

/**
 * @brief Create an array literal expression ( [ expr, expr, ... ] ).
 * @param elements Array of expressions.
 * @param count Number of elements.
 * @return New expression, or NULL on error.
 */
cco_expr_t* cco_expr_array(cco_expr_t **elements, size_t count);

/**
 * @brief Create a template instantiation expression (#TemplateName(args)).
 * @param template_name Name of the template.
 * @param positional_args Array of argument expressions (positional). May be NULL if arg_count == 0.
 * @param arg_count Number of positional arguments.
 * @param named_args Field initializer list for named instantiation (mutually exclusive with positional). May be NULL.
 * @return New expression, or NULL on error.
 * @note Exactly one of @p positional_args or @p named_args must be non-NULL.
 */
cco_expr_t* cco_expr_instantiate(const char *template_name,
                                 cco_expr_t **positional_args, size_t arg_count,
                                 cco_field_init_list_t *named_args);

/**
 * @brief Create a static method call expression (#TemplateName:method(args)).
 * @param template_name Name of the template.
 * @param method_name Name of the static method.
 * @param args Array of argument expressions.
 * @param arg_count Number of arguments.
 * @return New expression, or NULL on error.
 */
cco_expr_t* cco_expr_static_call(const char *template_name, const char *method_name,
                                 cco_expr_t **args, size_t arg_count);

/**
 * @brief Create a $format built-in call expression.
 * @param string_expr Expression that must evaluate to a string.
 * @return New expression, or NULL on error.
 * @note The argument is evaluated at runtime; the string is then parsed as a CCO expression
 *       using the current symbol table, and the result is returned.
 */
cco_expr_t* cco_expr_format(cco_expr_t *string_expr);

/**
 * @brief Create a compound expression $( ... ) with operators.
 * @param inner The expression inside the parentheses.
 * @return New expression, or NULL on error.
 * @note This is used to explicitly mark an expression context where operators are allowed.
 */
cco_expr_t* cco_expr_compound(cco_expr_t *inner);

/**
 * @brief Create a binary operation expression.
 * @param lhs Left operand.
 * @param op Operator.
 * @param rhs Right operand.
 * @return New expression, or NULL on error.
 */
cco_expr_t* cco_expr_binary(cco_expr_t *lhs, cco_op_t op, cco_expr_t *rhs);

/**
 * @brief Create a unary operation expression.
 * @param op Operator (only CCO_OP_NOT for now).
 * @param operand Operand.
 * @return New expression, or NULL on error.
 */
cco_expr_t* cco_expr_unary(cco_op_t op, cco_expr_t *operand);

/**
 * @brief Create a coalescing expression (left | right).
 * This is a special binary operator with short-circuit semantics.
 * @param left Left operand.
 * @param right Right operand.
 * @return New expression, or NULL on error.
 */
cco_expr_t* cco_expr_coalesce(cco_expr_t *left, cco_expr_t *right);

/**
 * @brief Free an expression tree.
 * @param expr Expression to free (may be NULL).
 */
void cco_expr_free(cco_expr_t *expr);

/**
 * @brief Evaluate an expression in a given context.
 * @param expr Expression to evaluate.
 * @param st Symbol table for type lookups.
 * @param self For instance methods, the current instance (can be NULL for static contexts).
 * @param locals A map of variable name -> value for local bindings (may be NULL).
 * @return Resulting value object, or NULL on error. Caller owns the result.
 */
cco_object_t* cco_expr_eval(const cco_expr_t *expr, cco_symbol_table_t *st,
                            cco_object_t *self, cco_object_t *locals);

/* Convenience wrappers for direct instantiation and static calls */

/**
 * @brief Directly instantiate a template with positional arguments.
 * @param st Symbol table (must contain the template definition).
 * @param template_name Name of the template.
 * @param args Array of argument values (cco_object_t*), ownership NOT transferred.
 * @param arg_count Number of arguments.
 * @return New template instance, or NULL on error. Caller owns the returned object.
 * @note This is a convenience wrapper around cco_expr_instantiate() + cco_expr_eval().
 */
cco_object_t* cco_instantiate(cco_symbol_table_t *st, const char *template_name,
                              cco_object_t **args, size_t arg_count);

/**
 * @brief Directly call a static method.
 * @param st Symbol table.
 * @param template_name Name of the template.
 * @param method_name Name of the static method.
 * @param args Array of argument values (cco_object_t*), ownership NOT transferred.
 * @param arg_count Number of arguments.
 * @return Return value of the static method, or NULL on error.
 */
cco_object_t* cco_call_static(cco_symbol_table_t *st, const char *template_name,
                              const char *method_name, cco_object_t **args, size_t arg_count);

/*==============================================================================
 * Field Initializer List (for named instantiation)
 *============================================================================*/

/**
 * @brief Create an empty field initializer list.
 * @return New list, or NULL on allocation failure.
 */
cco_field_init_list_t* cco_field_init_list_new(void);

/**
 * @brief Add a field initializer (field: value).
 * @param list List.
 * @param field_name Field name.
 * @param value_expr Expression for the value.
 * @return 0 on success, -1 on error.
 */
int cco_field_init_list_add(cco_field_init_list_t *list, const char *field_name, cco_expr_t *value_expr);

/**
 * @brief Free a field initializer list.
 * @param list List to free (may be NULL).
 */
void cco_field_init_list_free(cco_field_init_list_t *list);

/*==============================================================================
 * Enhanced Parsing (Full Language)
 *============================================================================*/

/**
 * @brief Parse a .cco source string into a symbol table and root value.
 *
 * This is the main entry point for the full language. It parses all top-level
 * declarations ($typedef, $enum, $temp), builds a symbol table, then evaluates
 * the root expression and returns both.
 *
 * @param text Null-terminated .cco source text.
 * @return Parse result containing symbol table and root value, or NULL on error.
 *         Must be freed with cco_parse_result_free().
 * @see cco_last_error() for error details.
 */
cco_parse_result_t* cco_parse_full(const char *text);

/**
 * @brief Parse a .cco file into a parse result.
 * @param filename Path to the file.
 * @return Parse result, or NULL on error.
 */
cco_parse_result_t* cco_parse_full_from_file(const char *filename);

/**
 * @brief Parse a .cco stream into a parse result.
 * @param fp Open FILE stream (must be readable).
 * @return Parse result, or NULL on error.
 */
cco_parse_result_t* cco_parse_full_from_stream(FILE *fp);

/**
 * @brief Get the symbol table from a parse result.
 * @param res Parse result (must not be NULL).
 * @return Symbol table (owned by the result, do not free separately).
 */
cco_symbol_table_t* cco_parse_result_symbols(cco_parse_result_t *res);

/**
 * @brief Get the root value from a parse result.
 * @param res Parse result (must not be NULL).
 * @return Root value (owned by the result, do not free separately).
 */
cco_object_t* cco_parse_result_root(cco_parse_result_t *res);

/**
 * @brief Free a parse result and all its contents.
 * @param res Parse result to free (may be NULL).
 */
void cco_parse_result_free(cco_parse_result_t *res);

/**
 * @brief Legacy simple parse (returns only the root value, ignores declarations).
 * @deprecated Use cco_parse_full() for spec compliance.
 * @param text Null-terminated .cco source text.
 * @return Root value, or NULL on error.
 */
cco_object_t* cco_parse(const char *text);

/**
 * @brief Legacy parse from file (ignores declarations).
 * @deprecated Use cco_parse_full_from_file().
 */
cco_object_t* cco_parse_from_file(const char *filename);

/**
 * @brief Legacy parse from stream (ignores declarations).
 * @deprecated Use cco_parse_full_from_stream().
 */
cco_object_t* cco_parse_from_stream(FILE *fp);

/*==============================================================================
 * Serialization (Extend to handle full language)
 *============================================================================*/

/**
 * @brief Serialize a value to compact .cco string (value-only, ignores types).
 * @param obj Object to serialize.
 * @return Allocated string, must be freed by caller with free(), or NULL on error.
 */
char* cco_serialize(const cco_object_t *obj);

/**
 * @brief Serialize a value to a pretty-printed .cco string.
 * @param obj Object to serialize.
 * @param indent Number of spaces per indentation level (0 = compact).
 * @return Allocated string, must be freed by caller with free(), or NULL on error.
 */
char* cco_serialize_pretty(const cco_object_t *obj, int indent);

/**
 * @brief Write an object to a file in compact format.
 * @param obj Object to write.
 * @param filename Output file path.
 * @return 0 on success, -1 on error.
 */
int cco_save_file(const cco_object_t *obj, const char *filename);

/**
 * @brief Write an object to a file in pretty-printed format.
 * @param obj Object to write.
 * @param filename Output file path.
 * @param indent Number of spaces per indentation level.
 * @return 0 on success, -1 on error.
 */
int cco_save_file_pretty(const cco_object_t *obj, const char *filename, int indent);

/**
 * @brief Write an object to a stream in compact format.
 * @param obj Object to write.
 * @param fp Output stream (must be writable).
 * @return 0 on success, -1 on error.
 */
int cco_save_stream(const cco_object_t *obj, FILE *fp);

/**
 * @brief Write an object to a stream in pretty-printed format.
 * @param obj Object to write.
 * @param fp Output stream.
 * @param indent Number of spaces per indentation level.
 * @return 0 on success, -1 on error.
 */
int cco_save_stream_pretty(const cco_object_t *obj, FILE *fp, int indent);

/**
 * @brief Serialize a full parse result (including type annotations, templates, etc.)
 *        – a future extension.
 * @param res Parse result.
 * @return Allocated string, or NULL on error.
 * @note Not yet implemented; reserved for future use.
 */
char* cco_serialize_full(const cco_parse_result_t *res);

/**
 * @brief Pretty‑print serialize a full parse result.
 * @param res Parse result.
 * @param indent Number of spaces per indentation level.
 * @return Allocated string, or NULL on error.
 * @note Not yet implemented; reserved for future use.
 */
char* cco_serialize_full_pretty(const cco_parse_result_t *res, int indent);

/*==============================================================================
 * Error Handling
 *============================================================================*/

/**
 * @brief Structured diagnostic (error or warning) with source location.
 */
typedef struct cco_diag {
    const char *file;       /**< Source filename */
    size_t      line;       /**< 1-based line number */
    size_t      col;        /**< 1-based column number */
    const char *message;    /**< Human-readable error/warning message */
    int         is_error;   /**< Non-zero for error, zero for warning */
} cco_diag_t;

/**
 * @brief Get the number of diagnostics from the last failed operation.
 * @return Diagnostic count (0 if no diagnostics).
 */
int cco_diag_count(void);

/**
 * @brief Get a diagnostic by index.
 * @param i Index (0 .. cco_diag_count()-1).
 * @return Pointer to diagnostic, or NULL if index out of range.
 */
const cco_diag_t* cco_diag_get(int i);

/**
 * @brief Print a diagnostic with source context to stderr.
 *
 * Shows the source line and a caret (^) marking the error column.
 * Example:
 *   basic.cco:5:10: unexpected identifier 'FOO'
 *      |
 *    5 |     mode: FOO,
 *      |           ^~~
 *
 * @param diag Diagnostic to print.
 * @param source The original source text (may be NULL for no context).
 */
void cco_diag_print(const cco_diag_t *diag, const char *source);

/**
 * @brief Print all pending diagnostics to stderr.
 * @param source The original source text (may be NULL).
 */
void cco_diag_print_all(const char *source);

/**
 * @brief Get the last error message (thread-local if CCO_THREADSAFE defined).
 * @deprecated Use cco_diag_count() / cco_diag_get() / cco_diag_print() instead.
 */
const char* cco_last_error(void);

/**
 * @brief Clear the last error and all diagnostics.
 */
void cco_clear_error(void);

#ifdef __cplusplus
}
#endif

#endif /* CCO_H */