/**
 * @file cco.hpp
 * @brief C++11 object-oriented wrapper for libcco – parsing, manipulating,
 *        and serializing .cco configuration files.
 *
 * This header provides RAII wrappers, scoped enumerations, type-safe accessors,
 * operator overloading, and STL integration for the complete CCO language
 * specification version 0.1.0.
 *
 * @par Usage
 * @code
 * #include <cnt/cco.hpp>
 * #include <iostream>
 *
 * int main() {
 *     auto res = cnt::parse("name: \"CCO\",\nversion: 1\n");
 *     cnt::Value root = res.root();
 *     std::cout << "name = " << root["name"] << std::endl;
 *     return 0;
 * }
 * @endcode
 *
 * @see cnt/cco.h for the underlying C API.
 */

#ifndef CCO_HPP
#define CCO_HPP

#include <cnt/cco.h>

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cnt {

/*============================================================================
 * Scoped Enumerations
 *============================================================================*/

enum class ValueType {
    None             = CCO_TYPE_NONE,
    String           = CCO_TYPE_STRING,
    Int              = CCO_TYPE_INT,
    Float            = CCO_TYPE_FLOAT,
    Bool             = CCO_TYPE_BOOL,
    TemplateInstance = CCO_TYPE_TEMPLATE_INSTANCE,
    Map              = CCO_TYPE_MAP,
    Array            = CCO_TYPE_ARRAY
};

enum class TypeKind {
    Primitive = CCO_TYPEKIND_PRIMITIVE,
    Array     = CCO_TYPEKIND_ARRAY,
    Object    = CCO_TYPEKIND_OBJECT,
    Named     = CCO_TYPEKIND_NAMED,
    Dyn       = CCO_TYPEKIND_DYN
};

enum class PrimitiveType {
    String  = CCO_PRIM_STRING,
    Integer = CCO_PRIM_INTEGER,
    Float   = CCO_PRIM_FLOAT,
    Boolean = CCO_PRIM_BOOLEAN,
    None    = CCO_PRIM_NONE
};

enum class Op {
    Add      = CCO_OP_ADD,
    Sub      = CCO_OP_SUB,
    Mul      = CCO_OP_MUL,
    Div      = CCO_OP_DIV,
    Eq       = CCO_OP_EQ,
    Ne       = CCO_OP_NE,
    Lt       = CCO_OP_LT,
    Gt       = CCO_OP_GT,
    Le       = CCO_OP_LE,
    Ge       = CCO_OP_GE,
    And      = CCO_OP_AND,
    Or       = CCO_OP_OR,
    Not      = CCO_OP_NOT,
    Coalesce = CCO_OP_COALESCE
};

enum class StmtKind {
    Bind   = CCO_STMT_BIND,
    Assign = CCO_STMT_ASSIGN,
    Return = CCO_STMT_RETURN,
    Expr   = CCO_STMT_EXPR
};

/*============================================================================
 * Forward Declarations
 *============================================================================*/

class Array;
class Value;
class ValueProxy;
class ArrayProxy;
class SymbolTable;
class TypeExpr;
class Template;
class Constructor;
class StaticMethod;
class ParamList;
class FieldInitList;
class MethodBody;
class Stmt;
class Expr;
class ParseResult;
struct Diagnostic;

/*============================================================================
 * Array
 *============================================================================*/

class Array {
    cco_array_t *arr_;

    void do_free() noexcept { if (arr_) cco_array_free(arr_); }

public:
    Array() : arr_(cco_array_new()) {}
    ~Array() { do_free(); }

    Array(const Array &other)
        : arr_(other.arr_ ? cco_array_copy(other.arr_) : nullptr) {}

    Array(Array &&other) noexcept : arr_(other.arr_) { other.arr_ = nullptr; }

    Array &operator=(Array other) noexcept {
        swap(*this, other);
        return *this;
    }

    friend void swap(Array &a, Array &b) noexcept {
        std::swap(a.arr_, b.arr_);
    }

    size_t size() const { return cco_array_size(arr_); }

    Value get(size_t index) const;
    ArrayProxy operator[](size_t index);
    Value operator[](size_t index) const;

    void add(const Value &value);
    void add(Value &&value);
    void insert(size_t index, const Value &value);
    void remove(size_t index);
    void clear() { cco_array_clear(arr_); }

    cco_array_t *raw() const { return arr_; }
    cco_array_t *release() noexcept {
        cco_array_t *p = arr_;
        arr_ = nullptr;
        return p;
    }

    static Array from_raw(cco_array_t *arr) {
        Array a;
        a.arr_ = arr;
        return a;
    }
};

/*============================================================================
 * ValueProxy
 *============================================================================*/

class ValueProxy {
    friend class Value;
    cco_object_t *obj_;
    std::string   key_;

    ValueProxy(cco_object_t *obj, const char *key)
        : obj_(obj), key_(key) {}

public:
    operator Value() const;
    operator const char *() const;
    operator std::string() const;
    operator long long() const;
    operator double() const;
    operator bool() const;

    friend std::ostream &operator<<(std::ostream &os, const ValueProxy &p);

    ValueProxy &operator=(Value value);
    Value operator[](const char *subkey) const;
    Value operator[](size_t index) const;
};

/*============================================================================
 * ArrayProxy
 *============================================================================*/

class ArrayProxy {
    friend class Value;
    friend class Array;
    cco_array_t *arr_;
    size_t       index_;

    ArrayProxy(cco_array_t *arr, size_t index) : arr_(arr), index_(index) {}

public:
    operator Value() const;
    operator const char *() const;
    operator std::string() const;
    operator long long() const;
    operator double() const;
    operator bool() const;

    friend std::ostream &operator<<(std::ostream &os, const ArrayProxy &p);

    ArrayProxy &operator=(Value value);
    Value operator[](const char *key) const;
    Value operator[](size_t index) const;
};

/*============================================================================
 * Value
 *============================================================================*/

class Value {
    cco_object_t *obj_;

    void do_free() noexcept { if (obj_) cco_object_free(obj_); }

public:
    /*------ Named constructors ------*/

    static Value make_none()                { return Value(cco_none_new()); }

    static Value make_string(const char *s) {
        if (!s) throw std::invalid_argument("string must not be null");
        return Value(cco_string_new(s));
    }

    static Value make_string(const std::string &s) {
        return make_string(s.c_str());
    }

    static Value make_int(long long i)      { return Value(cco_int_new(i)); }
    static Value make_float(double f)       { return Value(cco_float_new(f)); }

    static Value make_bool(bool b) {
        return Value(cco_bool_new(b ? 1 : 0));
    }

    static Value make_map()                 { return Value(cco_object_new()); }

    static Value make_array(Array arr) {
        return Value(cco_array_wrap(arr.release()));
    }

    static Value make_template_instance(const std::string &name) {
        return Value(cco_template_instance_new(name.c_str()));
    }

    /** @brief Create from owned raw C pointer. */
    static Value from_raw(cco_object_t *obj) { return Value(obj); }

    /*------ Constructors / destructor / assignment ------*/

    Value() : obj_(cco_none_new()) {}

    /** @brief Direct constructor from owned C pointer. */
    explicit Value(cco_object_t *obj) : obj_(obj) {}
    ~Value() { do_free(); }

    Value(const Value &other)
        : obj_(other.obj_ ? cco_object_copy(other.obj_) : cco_none_new()) {}

    Value(Value &&other) noexcept : obj_(other.obj_) {
        other.obj_ = cco_none_new();
    }

    Value &operator=(Value other) noexcept {
        swap(*this, other);
        return *this;
    }

    friend void swap(Value &a, Value &b) noexcept {
        std::swap(a.obj_, b.obj_);
    }

    /*------ Type queries ------*/

    ValueType type() const noexcept {
        return static_cast<ValueType>(cco_object_type(obj_));
    }

    bool is_none()               const noexcept { return type() == ValueType::None; }
    bool is_string()             const noexcept { return type() == ValueType::String; }
    bool is_int()                const noexcept { return type() == ValueType::Int; }
    bool is_float()              const noexcept { return type() == ValueType::Float; }
    bool is_bool()               const noexcept { return type() == ValueType::Bool; }
    bool is_map()                const noexcept { return type() == ValueType::Map; }
    bool is_array()              const noexcept { return type() == ValueType::Array; }
    bool is_template_instance()  const noexcept { return type() == ValueType::TemplateInstance; }

    /*------ Implicit conversion operators ------*/

    explicit operator const char *() const {
        if (!is_string()) throw std::logic_error("Value is not a String");
        return cco_string_get(obj_);
    }

    operator std::string() const { return std::string(static_cast<const char *>(*this)); }

    operator long long() const {
        if (!is_int()) throw std::logic_error("Value is not an Int");
        return cco_int_get(obj_);
    }

    operator double() const {
        if (!is_float()) throw std::logic_error("Value is not a Float");
        return cco_float_get(obj_);
    }

    operator bool() const {
        if (!is_bool()) throw std::logic_error("Value is not a Bool");
        return cco_bool_get(obj_) != 0;
    }

    /*------ Explicit typed accessors ------*/

    const char *c_str() const   { return static_cast<const char *>(*this); }
    std::string as_string() const { return *this; }
    long long as_int() const    { return *this; }
    double as_float() const     { return *this; }
    bool as_bool() const        { return *this; }

    /*------ Map interface ------*/

    ValueProxy operator[](const char *key) {
        if (!cco_object_has_key(obj_, key))
            throw std::out_of_range(std::string("key not found: ") + key);
        return ValueProxy(obj_, key);
    }

    Value operator[](const char *key) const {
        cco_object_t *v = cco_object_get(obj_, key);
        if (!v) throw std::out_of_range(std::string("key not found: ") + key);
        return Value(cco_object_copy(v));
    }

    ValueProxy operator[](const std::string &key) { return (*this)[key.c_str()]; }
    Value operator[](const std::string &key) const { return (*this)[key.c_str()]; }

    bool has(const char *key) const {
        return cco_object_has_key(obj_, key) != 0;
    }

    size_t map_size() const { return cco_object_size(obj_); }

    void set(const char *key, Value value) {
        if (cco_object_set(obj_, key, value.release()) != 0)
            throw std::runtime_error(std::string("failed to set key: ") + key);
    }

    void del(const char *key) {
        if (cco_object_del(obj_, key) != 0)
            throw std::out_of_range(std::string("key not found: ") + key);
    }

    void map_clear() { cco_object_clear(obj_); }

    /*------ Array interface ------*/

    size_t array_size() const {
        const cco_array_t *a = cco_object_as_array(obj_);
        return a ? cco_array_size(a) : 0;
    }

    ArrayProxy operator[](size_t index) {
        cco_array_t *a = const_cast<cco_array_t *>(cco_object_as_array(obj_));
        if (!a) throw std::logic_error("Value is not an Array");
        if (index >= array_size())
            throw std::out_of_range("index out of range");
        return ArrayProxy(a, index);
    }

    Value operator[](size_t index) const {
        const cco_array_t *a = cco_object_as_array(obj_);
        if (!a) throw std::logic_error("Value is not an Array");
        cco_object_t *v = cco_array_get(a, index);
        if (!v) throw std::out_of_range("index out of range");
        return Value(cco_object_copy(v));
    }

    /*------ Template instance ------*/

    std::string template_name() const {
        if (!is_template_instance())
            throw std::logic_error("Value is not a TemplateInstance");
        const char *n = cco_object_template_name(obj_);
        return n ? std::string(n) : std::string();
    }

    /*------ Serialization ------*/

    std::string serialize() const {
        char *s = cco_serialize(obj_);
        if (!s) return std::string();
        std::string r(s);
        std::free(s);
        return r;
    }

    std::string serialize_pretty(int indent = 2) const {
        char *s = cco_serialize_pretty(obj_, indent);
        if (!s) return std::string();
        std::string r(s);
        std::free(s);
        return r;
    }

    void save(const std::string &filename) const {
        if (cco_save_file(obj_, filename.c_str()) != 0)
            throw std::runtime_error("failed to save: " + filename);
    }

    void save_pretty(const std::string &filename, int indent = 2) const {
        if (cco_save_file_pretty(obj_, filename.c_str(), indent) != 0)
            throw std::runtime_error("failed to save: " + filename);
    }

    friend std::ostream &operator<<(std::ostream &os, const Value &v) {
        return os << v.serialize();
    }

    /*------ Comparison ------*/

    bool operator==(const Value &rhs) const {
        if (type() != rhs.type()) return false;
        switch (type()) {
        case ValueType::None:  return true;
        case ValueType::String:
            return std::strcmp(cco_string_get(obj_),
                               cco_string_get(rhs.obj_)) == 0;
        case ValueType::Int:
            return cco_int_get(obj_) == cco_int_get(rhs.obj_);
        case ValueType::Float:
            return cco_float_get(obj_) == cco_float_get(rhs.obj_);
        case ValueType::Bool:
            return (cco_bool_get(obj_) != 0) == (cco_bool_get(rhs.obj_) != 0);
        default:
            return serialize() == rhs.serialize();
        }
    }

    bool operator!=(const Value &rhs) const { return !(*this == rhs); }

    /*------ Iteration ------*/

    class Iterator;

    Iterator begin() const;
    Iterator end() const;

    /*------ Raw pointer management ------*/

    cco_object_t *raw() const { return obj_; }

    cco_object_t *release() noexcept {
        cco_object_t *p = obj_;
        obj_ = cco_none_new();
        return p;
    }
};

/*============================================================================
 * Value::Iterator – map iterator (defined after Value is complete)
 *============================================================================*/

class Value::Iterator {
public:
    struct Pair {
        const char *first;
        Value       second;
    };

    Iterator() : iter_(nullptr), pair_{nullptr, Value()} {}
    ~Iterator() { cco_map_iter_free(iter_); }

    Iterator(Iterator &&other) noexcept
        : iter_(other.iter_), pair_(other.pair_) {
        other.iter_ = nullptr;
        other.pair_.first = nullptr;
    }

    Iterator(const Iterator &) = delete;
    Iterator &operator=(const Iterator &) = delete;

    Iterator &operator++() {
        if (!iter_) return *this;
        const char *k = nullptr;
        cco_object_t *v = nullptr;
        if (cco_map_iter_next(iter_, &k, &v)) {
            pair_.first  = k;
            pair_.second = Value(cco_object_copy(v));
        } else {
            cco_map_iter_free(iter_);
            iter_ = nullptr;
            pair_.first = nullptr;
            pair_.second = Value();
        }
        return *this;
    }

    void operator++(int) { ++*this; }
    const Pair &operator*() const { return pair_; }
    const Pair *operator->() const { return &pair_; }

    bool operator==(const Iterator &rhs) const { return iter_ == rhs.iter_; }
    bool operator!=(const Iterator &rhs) const { return !(*this == rhs); }

private:
    friend class Value;
    cco_map_iter_t *iter_;
    Pair            pair_;

    explicit Iterator(cco_map_iter_t *iter)
        : iter_(iter), pair_{nullptr, Value()} {
        if (iter_) ++*this;
    }
};

inline Value::Iterator Value::begin() const {
    if (!is_map()) throw std::logic_error("Value is not a Map");
    cco_map_iter_t *iter = cco_map_iter_new(obj_);
    if (!iter) throw std::runtime_error("failed to create map iterator");
    return Iterator(iter);
}

inline Value::Iterator Value::end() const {
    return Iterator();
}

/*============================================================================
 * ValueProxy / ArrayProxy inline impls
 *============================================================================*/

inline ValueProxy::operator Value() const {
    cco_object_t *v = cco_object_get(obj_, key_.c_str());
    if (!v) throw std::out_of_range("key not found: " + key_);
    return Value(cco_object_copy(v));
}

inline ValueProxy &ValueProxy::operator=(Value value) {
    if (cco_object_set(obj_, key_.c_str(), value.release()) != 0)
        throw std::runtime_error("failed to set value for key: " + key_);
    return *this;
}

inline ValueProxy::operator const char *() const {
    return static_cast<const char *>(static_cast<Value>(*this));
}

inline ValueProxy::operator std::string() const {
    return static_cast<Value>(*this);
}

inline ValueProxy::operator long long() const {
    return static_cast<Value>(*this);
}

inline ValueProxy::operator double() const {
    return static_cast<Value>(*this);
}

inline ValueProxy::operator bool() const {
    return static_cast<Value>(*this);
}

inline Value ValueProxy::operator[](const char *subkey) const {
    return static_cast<Value>(*this)[subkey];
}

inline Value ValueProxy::operator[](size_t index) const {
    return static_cast<Value>(*this)[index];
}

inline ArrayProxy::operator Value() const {
    cco_object_t *v = cco_array_get(arr_, index_);
    if (!v) throw std::out_of_range("index out of range");
    return Value(cco_object_copy(v));
}

inline ArrayProxy &ArrayProxy::operator=(Value value) {
    cco_object_t *v = cco_array_get(arr_, index_);
    if (!v) throw std::out_of_range("index out of range");
    cco_array_remove(arr_, index_);
    if (cco_array_insert(arr_, index_, value.release()) != 0)
        throw std::runtime_error("failed to replace array element");
    return *this;
}

inline ArrayProxy::operator const char *() const {
    return static_cast<const char *>(static_cast<Value>(*this));
}

inline ArrayProxy::operator std::string() const {
    return static_cast<Value>(*this);
}

inline ArrayProxy::operator long long() const {
    return static_cast<Value>(*this);
}

inline ArrayProxy::operator double() const {
    return static_cast<Value>(*this);
}

inline ArrayProxy::operator bool() const {
    return static_cast<Value>(*this);
}

inline Value ArrayProxy::operator[](const char *key) const {
    return static_cast<Value>(*this)[key];
}

inline Value ArrayProxy::operator[](size_t index) const {
    return static_cast<Value>(*this)[index];
}

inline std::ostream &operator<<(std::ostream &os, const ValueProxy &p) {
    return os << static_cast<Value>(p);
}

inline std::ostream &operator<<(std::ostream &os, const ArrayProxy &p) {
    return os << static_cast<Value>(p);
}

/*============================================================================
 * Array inline impls
 *============================================================================*/

inline Value Array::get(size_t index) const {
    cco_object_t *v = cco_array_get(arr_, index);
    if (!v) throw std::out_of_range("index out of range");
    return Value(cco_object_copy(v));
}

inline ArrayProxy Array::operator[](size_t index) {
    if (index >= cco_array_size(arr_))
        throw std::out_of_range("index out of range");
    return ArrayProxy(arr_, index);
}

inline Value Array::operator[](size_t index) const { return get(index); }

inline void Array::add(const Value &value) {
    cco_object_t *copy = cco_object_copy(value.raw());
    if (!copy) throw std::bad_alloc();
    if (cco_array_add(arr_, copy) != 0) {
        cco_object_free(copy);
        throw std::runtime_error("failed to add to array");
    }
}

inline void Array::add(Value &&value) {
    if (cco_array_add(arr_, value.release()) != 0)
        throw std::runtime_error("failed to add to array");
}

inline void Array::insert(size_t index, const Value &value) {
    cco_object_t *copy = cco_object_copy(value.raw());
    if (!copy) throw std::bad_alloc();
    if (cco_array_insert(arr_, index, copy) != 0) {
        cco_object_free(copy);
        throw std::runtime_error("failed to insert in array");
    }
}

inline void Array::remove(size_t index) {
    if (cco_array_remove(arr_, index) != 0)
        throw std::out_of_range("index out of range");
}

/*============================================================================
 * SymbolTable
 *============================================================================*/

class SymbolTable {
    cco_symbol_table_t *st_;

    void do_free() noexcept { if (st_) cco_symbol_table_free(st_); }

public:
    SymbolTable() : st_(cco_symbol_table_new()) {}
    ~SymbolTable() { do_free(); }

    SymbolTable(SymbolTable &&other) noexcept : st_(other.st_) {
        other.st_ = nullptr;
    }

    SymbolTable(const SymbolTable &) = delete;
    SymbolTable &operator=(const SymbolTable &) = delete;

    void add_typedef(const std::string &name, cco_type_expr_t *type) {
        if (cco_symbol_table_add_typedef(st_, name.c_str(), type) != 0)
            throw std::runtime_error("failed to add typedef: " + name);
    }

    void add_enum(const std::string &name,
                  const std::vector<std::string> &values) {
        std::vector<const char *> cvals;
        cvals.reserve(values.size() + 1);
        for (const auto &v : values) cvals.push_back(v.c_str());
        cvals.push_back(nullptr);
        if (cco_symbol_table_add_enum(st_, name.c_str(), cvals.data()) != 0)
            throw std::runtime_error("failed to add enum: " + name);
    }

    void add_template(cco_template_t *tmpl) {
        if (cco_symbol_table_add_template(st_, tmpl) != 0)
            throw std::runtime_error("failed to add template");
    }

    cco_type_expr_t *lookup_type(const std::string &name) const {
        return cco_symbol_table_lookup_type(st_, name.c_str());
    }

    bool has_enum_value(const std::string &enum_name,
                        const std::string &value_name) const {
        return cco_symbol_table_has_enum_value(
            st_, enum_name.c_str(), value_name.c_str()) != 0;
    }

    cco_template_t *lookup_template(const std::string &name) const {
        return cco_symbol_table_lookup_template(st_, name.c_str());
    }

    size_t count() const { return cco_symtab_get_count(st_); }
    const char *name_at(size_t idx) const {
        return cco_symtab_get_name(st_, idx);
    }

    cco_symbol_table_t *raw() const { return st_; }

    static SymbolTable from_raw(cco_symbol_table_t *st) {
        SymbolTable t;
        t.st_ = st;
        return t;
    }
};

/*============================================================================
 * TypeExpr
 *============================================================================*/

class TypeExpr {
    cco_type_expr_t *te_;

    void do_free() noexcept { if (te_) cco_type_expr_free(te_); }

    explicit TypeExpr(cco_type_expr_t *te) : te_(te) {}

public:
    TypeExpr() : te_(nullptr) {}

    explicit TypeExpr(PrimitiveType prim)
        : te_(cco_type_primitive(static_cast<cco_primitive_type_t>(prim))) {}

    explicit TypeExpr(const std::string &name)
        : te_(cco_type_named(name.c_str())) {}

    static TypeExpr dyn(const std::string &base_name) {
        return TypeExpr(cco_type_dyn(base_name.c_str()));
    }

    ~TypeExpr() { do_free(); }

    TypeExpr(TypeExpr &&other) noexcept : te_(other.te_) {
        other.te_ = nullptr;
    }

    TypeExpr(const TypeExpr &) = delete;
    TypeExpr &operator=(const TypeExpr &) = delete;

    TypeKind kind() const {
        return static_cast<TypeKind>(cco_type_expr_kind(te_));
    }

    PrimitiveType primitive_type() const {
        return static_cast<PrimitiveType>(cco_type_expr_primitive(te_));
    }

    const char *name() const { return cco_type_expr_name(te_); }

    TypeExpr clone() const { return TypeExpr(cco_type_expr_copy(te_)); }

    bool equals(const TypeExpr &rhs) const {
        return cco_type_expr_equal(te_, rhs.te_) != 0;
    }

    cco_type_expr_t *raw() const { return te_; }
    cco_type_expr_t *release() noexcept {
        cco_type_expr_t *p = te_;
        te_ = nullptr;
        return p;
    }

    static TypeExpr from_raw(cco_type_expr_t *te) {
        TypeExpr e;
        e.te_ = te;
        return e;
    }
};

/*============================================================================
 * ParamList
 *============================================================================*/

class ParamList {
    cco_param_list_t *pl_;

    void do_free() noexcept { if (pl_) cco_param_list_free(pl_); }

public:
    ParamList() : pl_(nullptr) {}

    ParamList(const std::vector<const char *> &names,
              std::vector<cco_type_expr_t *> &types)
        : pl_(nullptr) {
        size_t n = names.size();
        pl_ = cco_param_list_new(
            n,
            n ? const_cast<const char **>(names.data()) : nullptr,
            n ? types.data() : nullptr);
        if (!pl_) throw std::bad_alloc();
    }

    ~ParamList() { do_free(); }

    ParamList(ParamList &&other) noexcept : pl_(other.pl_) {
        other.pl_ = nullptr;
    }

    ParamList(const ParamList &) = delete;
    ParamList &operator=(const ParamList &) = delete;

    cco_param_list_t *raw() const { return pl_; }
    cco_param_list_t *release() noexcept {
        cco_param_list_t *p = pl_;
        pl_ = nullptr;
        return p;
    }

    static ParamList from_raw(cco_param_list_t *pl) {
        ParamList p;
        p.pl_ = pl;
        return p;
    }
};

/*============================================================================
 * FieldInitList
 *============================================================================*/

class FieldInitList {
    cco_field_init_list_t *fl_;

    void do_free() noexcept { if (fl_) cco_field_init_list_free(fl_); }

public:
    FieldInitList() : fl_(cco_field_init_list_new()) {}
    ~FieldInitList() { do_free(); }

    FieldInitList(FieldInitList &&other) noexcept : fl_(other.fl_) {
        other.fl_ = nullptr;
    }

    FieldInitList(const FieldInitList &) = delete;
    FieldInitList &operator=(const FieldInitList &) = delete;

    void add(const std::string &field_name, cco_expr_t *value_expr) {
        if (cco_field_init_list_add(fl_, field_name.c_str(), value_expr) != 0)
            throw std::runtime_error("failed to add field initializer");
    }

    cco_field_init_list_t *raw() const { return fl_; }
    cco_field_init_list_t *release() noexcept {
        cco_field_init_list_t *p = fl_;
        fl_ = nullptr;
        return p;
    }

    static FieldInitList from_raw(cco_field_init_list_t *fl) {
        FieldInitList f;
        f.fl_ = fl;
        return f;
    }
};

/*============================================================================
 * Stmt
 *============================================================================*/

class Stmt {
    cco_stmt_t *stmt_;

    void do_free() noexcept { if (stmt_) cco_stmt_free(stmt_); }

    explicit Stmt(cco_stmt_t *s) : stmt_(s) {}

public:
    Stmt() : stmt_(nullptr) {}

    static Stmt bind(const char *name, cco_type_expr_t *type,
                     cco_expr_t *value) {
        return Stmt(cco_stmt_bind(name, type, value));
    }

    static Stmt assign(const char *field_name, cco_expr_t *value) {
        return Stmt(cco_stmt_assign(field_name, value));
    }

    static Stmt ret(cco_type_expr_t *type, cco_expr_t *value) {
        return Stmt(cco_stmt_return(type, value));
    }

    static Stmt expr(cco_expr_t *value) {
        return Stmt(cco_stmt_expr(value));
    }

    ~Stmt() { do_free(); }

    Stmt(Stmt &&other) noexcept : stmt_(other.stmt_) {
        other.stmt_ = nullptr;
    }

    Stmt(const Stmt &) = delete;
    Stmt &operator=(const Stmt &) = delete;

    cco_stmt_t *raw() const { return stmt_; }
    cco_stmt_t *release() noexcept {
        cco_stmt_t *p = stmt_;
        stmt_ = nullptr;
        return p;
    }

    static Stmt from_raw(cco_stmt_t *s) {
        Stmt st;
        st.stmt_ = s;
        return st;
    }
};

/*============================================================================
 * MethodBody
 *============================================================================*/

class MethodBody {
    cco_method_body_t *mb_;

    void do_free() noexcept { if (mb_) cco_method_body_free(mb_); }

public:
    MethodBody() : mb_(cco_method_body_new()) {}
    ~MethodBody() { do_free(); }

    MethodBody(MethodBody &&other) noexcept : mb_(other.mb_) {
        other.mb_ = nullptr;
    }

    MethodBody(const MethodBody &) = delete;
    MethodBody &operator=(const MethodBody &) = delete;

    void add_stmt(cco_stmt_t *stmt) {
        if (cco_method_body_add_stmt(mb_, stmt) != 0)
            throw std::runtime_error("failed to add statement");
    }

    void set_final_expr(cco_expr_t *expr) {
        cco_method_body_set_final_expr(mb_, expr);
    }

    cco_method_body_t *raw() const { return mb_; }
    cco_method_body_t *release() noexcept {
        cco_method_body_t *p = mb_;
        mb_ = nullptr;
        return p;
    }

    static MethodBody from_raw(cco_method_body_t *mb) {
        MethodBody b;
        b.mb_ = mb;
        return b;
    }
};

/*============================================================================
 * Constructor
 *============================================================================*/

class Constructor {
    cco_constructor_t *ctor_;

    void do_free() noexcept { if (ctor_) cco_constructor_free(ctor_); }

public:
    Constructor() : ctor_(cco_constructor_default()) {}

    Constructor(cco_param_list_t *params, cco_method_body_t *body)
        : ctor_(cco_constructor_new(params, body)) {}

    ~Constructor() { do_free(); }

    Constructor(Constructor &&other) noexcept : ctor_(other.ctor_) {
        other.ctor_ = nullptr;
    }

    Constructor(const Constructor &) = delete;
    Constructor &operator=(const Constructor &) = delete;

    cco_constructor_t *raw() const { return ctor_; }
    cco_constructor_t *release() noexcept {
        cco_constructor_t *p = ctor_;
        ctor_ = nullptr;
        return p;
    }

    static Constructor from_raw(cco_constructor_t *c) {
        Constructor ct;
        ct.ctor_ = c;
        return ct;
    }
};

/*============================================================================
 * StaticMethod
 *============================================================================*/

class StaticMethod {
    cco_static_method_t *sm_;

    void do_free() noexcept { if (sm_) cco_static_method_free(sm_); }

public:
    StaticMethod() : sm_(nullptr) {}

    StaticMethod(const char *name, int is_private,
                 const cco_generic_param_t *generic_params, size_t generic_count,
                 cco_param_list_t *params, cco_type_expr_t *return_type,
                 cco_method_body_t *body)
        : sm_(cco_static_method_new(name, is_private,
                                     generic_params, generic_count,
                                     params, return_type, body)) {}

    ~StaticMethod() { do_free(); }

    StaticMethod(StaticMethod &&other) noexcept : sm_(other.sm_) {
        other.sm_ = nullptr;
    }

    StaticMethod(const StaticMethod &) = delete;
    StaticMethod &operator=(const StaticMethod &) = delete;

    cco_static_method_t *raw() const { return sm_; }
    cco_static_method_t *release() noexcept {
        cco_static_method_t *p = sm_;
        sm_ = nullptr;
        return p;
    }

    static StaticMethod from_raw(cco_static_method_t *s) {
        StaticMethod sm;
        sm.sm_ = s;
        return sm;
    }
};

/*============================================================================
 * Template
 *============================================================================*/

class Template {
    cco_template_t *tmpl_;

    void do_free() noexcept { if (tmpl_) cco_template_free(tmpl_); }

public:
    Template() : tmpl_(nullptr) {}

    Template(const std::string &name, const std::string &parent = "")
        : tmpl_(cco_template_new(name.c_str(),
                                 parent.empty() ? nullptr : parent.c_str())) {}

    ~Template() { do_free(); }

    Template(Template &&other) noexcept : tmpl_(other.tmpl_) {
        other.tmpl_ = nullptr;
    }

    Template(const Template &) = delete;
    Template &operator=(const Template &) = delete;

    void add_field(const char *name, cco_type_expr_t *type,
                   cco_expr_t *default_expr) {
        if (cco_template_add_field(tmpl_, name, type, default_expr) != 0)
            throw std::runtime_error("failed to add field: " +
                                     std::string(name));
    }

    void add_constructor(cco_constructor_t *ctor) {
        if (cco_template_add_constructor(tmpl_, ctor) != 0)
            throw std::runtime_error("failed to add constructor");
    }

    void add_static_method(cco_static_method_t *method) {
        if (cco_template_add_static_method(tmpl_, method) != 0)
            throw std::runtime_error("failed to add static method");
    }

    cco_template_t *get_parent(cco_symbol_table_t *st) const {
        return cco_template_get_parent(tmpl_, st);
    }

    const char *parent_name() const {
        return cco_template_get_parent_name(tmpl_);
    }

    size_t field_count() const { return cco_template_field_count(tmpl_); }
    const char *field_name(size_t idx) const {
        return cco_template_field_name(tmpl_, idx);
    }

    Value instantiate(cco_symbol_table_t *st, cco_object_t **args,
                      size_t arg_count, const char *template_name) const {
        cco_object_t *obj = cco_instantiate(st, template_name,
                                             args, arg_count);
        if (!obj) throw std::runtime_error("failed to instantiate template");
        return Value::from_raw(obj);
    }

    Value call_static(cco_symbol_table_t *st, const char *method_name,
                      cco_object_t **args, size_t arg_count,
                      const char *template_name) const {
        cco_object_t *obj = cco_call_static(st, template_name, method_name,
                                             args, arg_count);
        if (!obj) throw std::runtime_error("failed to call static method");
        return Value::from_raw(obj);
    }

    cco_template_t *raw() const { return tmpl_; }
    cco_template_t *release() noexcept {
        cco_template_t *p = tmpl_;
        tmpl_ = nullptr;
        return p;
    }

    static Template from_raw(cco_template_t *t) {
        Template tm;
        tm.tmpl_ = t;
        return tm;
    }
};

/*============================================================================
 * Expr
 *============================================================================*/

class Expr {
    cco_expr_t *expr_;

    void do_free() noexcept { if (expr_) cco_expr_free(expr_); }

    explicit Expr(cco_expr_t *e) : expr_(e) {}

public:
    Expr() : expr_(nullptr) {}

    static Expr literal(cco_object_t *val) {
        return Expr(cco_expr_literal(val));
    }

    static Expr identifier(const char *name) {
        return Expr(cco_expr_identifier(name));
    }

    static Expr this_ref() { return Expr(cco_expr_this()); }

    static Expr object(const char **field_names,
                       cco_expr_t **field_exprs, size_t count) {
        return Expr(cco_expr_object(field_names, field_exprs, count));
    }

    static Expr array(cco_expr_t **elements, size_t count) {
        return Expr(cco_expr_array(elements, count));
    }

    static Expr instantiate(const char *template_name,
                            cco_expr_t **positional_args, size_t arg_count,
                            cco_field_init_list_t *named_args) {
        return Expr(cco_expr_instantiate(template_name, positional_args,
                                         arg_count, named_args));
    }

    static Expr static_call(const char *template_name,
                            const char *method_name,
                            cco_expr_t **args, size_t arg_count) {
        return Expr(cco_expr_static_call(template_name, method_name,
                                         args, arg_count));
    }

    static Expr format(cco_expr_t *string_expr) {
        return Expr(cco_expr_format(string_expr));
    }

    static Expr compound(cco_expr_t *inner) {
        return Expr(cco_expr_compound(inner));
    }

    static Expr binary(cco_expr_t *lhs, Op op, cco_expr_t *rhs) {
        return Expr(cco_expr_binary(lhs, static_cast<cco_op_t>(op), rhs));
    }

    static Expr unary(Op op, cco_expr_t *operand) {
        return Expr(cco_expr_unary(static_cast<cco_op_t>(op), operand));
    }

    static Expr coalesce(cco_expr_t *left, cco_expr_t *right) {
        return Expr(cco_expr_coalesce(left, right));
    }

    ~Expr() { do_free(); }

    Expr(Expr &&other) noexcept : expr_(other.expr_) {
        other.expr_ = nullptr;
    }

    Expr(const Expr &) = delete;
    Expr &operator=(const Expr &) = delete;

    Value eval(cco_symbol_table_t *st, cco_object_t *self,
               cco_object_t *locals) const {
        cco_object_t *obj = cco_expr_eval(expr_, st, self, locals);
        if (!obj) throw std::runtime_error("expression evaluation failed");
        return Value::from_raw(obj);
    }

    cco_expr_t *raw() const { return expr_; }
    cco_expr_t *release() noexcept {
        cco_expr_t *p = expr_;
        expr_ = nullptr;
        return p;
    }

    static Expr from_raw(cco_expr_t *e) {
        Expr ex;
        ex.expr_ = e;
        return ex;
    }
};

/*============================================================================
 * ParseResult
 *============================================================================*/

class ParseResult {
    cco_parse_result_t *res_;

    void do_free() noexcept { if (res_) cco_parse_result_free(res_); }

public:
    explicit ParseResult(cco_parse_result_t *res) : res_(res) {}
    ~ParseResult() { do_free(); }

    ParseResult(ParseResult &&other) noexcept : res_(other.res_) {
        other.res_ = nullptr;
    }

    ParseResult(const ParseResult &) = delete;
    ParseResult &operator=(const ParseResult &) = delete;

    SymbolTable symbols() const {
        return SymbolTable::from_raw(cco_parse_result_symbols(res_));
    }

    Value root() const {
        return Value::from_raw(cco_parse_result_root(res_));
    }

    std::string serialize() const {
        char *s = cco_serialize_full(res_);
        if (!s) return std::string();
        std::string r(s);
        std::free(s);
        return r;
    }

    std::string serialize_pretty(int indent = 2) const {
        char *s = cco_serialize_full_pretty(res_, indent);
        if (!s) return std::string();
        std::string r(s);
        std::free(s);
        return r;
    }

    friend std::ostream &operator<<(std::ostream &os, const ParseResult &pr) {
        return os << pr.serialize();
    }
};

/*============================================================================
 * Diagnostic
 *============================================================================*/

struct Diagnostic {
    std::string file;
    size_t      line;
    size_t      col;
    std::string message;
    bool        is_error;

    void print(const char *source = nullptr) const {
        cco_diag_t d;
        d.file     = file.c_str();
        d.line     = line;
        d.col      = col;
        d.message  = message.c_str();
        d.is_error = is_error ? 1 : 0;
        cco_diag_print(&d, source);
    }
};

/*============================================================================
 * Free Functions
 *============================================================================*/

inline ParseResult parse(const std::string &text) {
    cco_parse_result_t *res = cco_parse_full(text.c_str());
    if (!res) {
        const char *err = cco_last_error();
        throw std::runtime_error(
            "parse failed: " + std::string(err ? err : "unknown error"));
    }
    return ParseResult(res);
}

inline ParseResult parse_file(const std::string &filename) {
    cco_parse_result_t *res = cco_parse_full_from_file(filename.c_str());
    if (!res) {
        const char *err = cco_last_error();
        throw std::runtime_error(
            "parse failed: " + std::string(err ? err : "unknown error"));
    }
    return ParseResult(res);
}

inline std::vector<Diagnostic> get_diagnostics() {
    int count = cco_diag_count();
    std::vector<Diagnostic> diags;
    diags.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        const cco_diag_t *d = cco_diag_get(i);
        if (d) {
            Diagnostic diag;
            diag.file     = d->file ? d->file : "";
            diag.line     = d->line;
            diag.col      = d->col;
            diag.message  = d->message ? d->message : "";
            diag.is_error = d->is_error != 0;
            diags.push_back(std::move(diag));
        }
    }
    return diags;
}

inline void print_diagnostics(const char *source = nullptr) {
    cco_diag_print_all(source);
}

inline void clear_diagnostics() {
    cco_clear_error();
}

inline std::string serialize(const Value &v) { return v.serialize(); }

inline std::string serialize_pretty(const Value &v, int indent = 2) {
    return v.serialize_pretty(indent);
}

} // namespace cnt

#endif // CCO_HPP
