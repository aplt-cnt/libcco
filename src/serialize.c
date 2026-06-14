#include <cnt/cco_p.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Dynamic string builder */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} sb_t;

static int sb_init(sb_t *sb) {
    sb->data = (char*)malloc(256);
    if (!sb->data) return -1;
    sb->data[0] = '\0';
    sb->len = 0;
    sb->cap = 256;
    return 0;
}

static int sb_grow(sb_t *sb, size_t needed) {
    if (sb->len + needed + 1 <= sb->cap) return 0;
    size_t new_cap = sb->cap ? sb->cap * 2 : 256;
    while (sb->len + needed + 1 > new_cap) new_cap *= 2;
    char *p = (char*)realloc(sb->data, new_cap);
    if (!p) return -1;
    sb->data = p;
    sb->cap  = new_cap;
    return 0;
}

static void sb_append(sb_t *sb, const char *s, size_t len) {
    if (sb_grow(sb, len) != 0) return;
    memcpy(sb->data + sb->len, s, len);
    sb->len += len;
    sb->data[sb->len] = '\0';
}

static void sb_append_c(sb_t *sb, char c) {
    if (sb_grow(sb, 1) != 0) return;
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
}

static void sb_append_str(sb_t *sb, const char *s) {
    sb_append(sb, s, strlen(s));
}

/*---------------------------------------------------------------------------*/

static void serialize_value(const cco_object_t *obj, sb_t *sb, int indent,
                            int depth, int compact);

static void serialize_string(const cco_object_t *obj, sb_t *sb) {
    const char *s = cco_string_get(obj);
    sb_append_c(sb, '"');
    for (const char *p = s; *p; p++) {
        unsigned char c = (unsigned char)*p;
        switch (c) {
            case '"':  sb_append_str(sb, "\\\""); break;
            case '\\': sb_append_str(sb, "\\\\"); break;
            case '\n': sb_append_str(sb, "\\n");  break;
            case '\r': sb_append_str(sb, "\\r");  break;
            case '\t': sb_append_str(sb, "\\t");  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\x%02X", c);
                    sb_append_str(sb, buf);
                } else {
                    sb_append_c(sb, c);
                }
                break;
        }
    }
    sb_append_c(sb, '"');
}

static void serialize_map(const cco_object_t *obj, sb_t *sb, int indent,
                          int depth, int compact) {
    if (obj->field_count == 0) {
        sb_append_str(sb, "()");
        return;
    }
    int single_line = compact || (obj->field_count == 1 && !indent);
    if (!single_line) {
        sb_append_str(sb, "(\n");
        for (size_t i = 0; i < obj->field_count; i++) {
            for (int j = 0; j < (depth + 1) * indent; j++) sb_append_c(sb, ' ');
            sb_append_str(sb, obj->fields[i].key);
            sb_append_str(sb, ": ");
            serialize_value(obj->fields[i].value, sb, indent, depth + 1, compact);
            if (i < obj->field_count - 1) sb_append_c(sb, ',');
            sb_append_c(sb, '\n');
        }
        for (int j = 0; j < depth * indent; j++) sb_append_c(sb, ' ');
        sb_append_c(sb, ')');
    } else {
        sb_append_str(sb, "( ");
        for (size_t i = 0; i < obj->field_count; i++) {
            sb_append_str(sb, obj->fields[i].key);
            sb_append_str(sb, ": ");
            serialize_value(obj->fields[i].value, sb, indent, depth + 1, 1);
            if (i < obj->field_count - 1) {
                sb_append_c(sb, ',');
                sb_append_c(sb, ' ');
            }
        }
        sb_append_str(sb, " )");
    }
}

static void serialize_array(const cco_object_t *obj, sb_t *sb, int indent,
                            int depth, int compact) {
    const cco_array_t *arr = cco_object_as_array(obj);
    if (!arr || arr->count == 0) {
        sb_append_str(sb, "()");
        return;
    }
    int single_line = compact || (arr->count == 1 && !indent);
    if (!single_line) {
        sb_append_str(sb, "(\n");
        for (size_t i = 0; i < arr->count; i++) {
            for (int j = 0; j < (depth + 1) * indent; j++) sb_append_c(sb, ' ');
            serialize_value(arr->items[i], sb, indent, depth + 1, compact);
            if (i < arr->count - 1) sb_append_c(sb, ',');
            sb_append_c(sb, '\n');
        }
        for (int j = 0; j < depth * indent; j++) sb_append_c(sb, ' ');
        sb_append_c(sb, ')');
    } else {
        sb_append_str(sb, "( ");
        for (size_t i = 0; i < arr->count; i++) {
            serialize_value(arr->items[i], sb, indent, depth + 1, 1);
            if (i < arr->count - 1) {
                sb_append_c(sb, ',');
                sb_append_c(sb, ' ');
            }
        }
        sb_append_str(sb, " )");
    }
}

static void serialize_value(const cco_object_t *obj, sb_t *sb, int indent,
                            int depth, int compact) {
    if (!obj) { sb_append_str(sb, "None"); return; }
    switch (obj->type) {
    case CCO_TYPE_NONE:
        sb_append_str(sb, "None");
        break;
    case CCO_TYPE_BOOL:
        sb_append_str(sb, obj->data.bool_val ? "true" : "false");
        break;
    case CCO_TYPE_INT: {
        char buf[32];
        snprintf(buf, sizeof(buf), "%lld", (long long)obj->data.int_val);
        sb_append_str(sb, buf);
        break;
    }
    case CCO_TYPE_FLOAT: {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.15g", obj->data.float_val);
        /* Ensure decimal point is present for float literals */
        if (!strchr(buf, '.') && !strchr(buf, 'e') && !strchr(buf, 'E')) {
            sb_append_str(sb, buf);
            sb_append_str(sb, ".0");
        } else {
            sb_append_str(sb, buf);
        }
        break;
    }
    case CCO_TYPE_STRING:
        serialize_string(obj, sb);
        break;
    case CCO_TYPE_ARRAY:
        serialize_array(obj, sb, indent, depth, compact);
        break;
    case CCO_TYPE_MAP:
    case CCO_TYPE_TEMPLATE_INSTANCE:
        /* For single-field maps at root level, could use shorthand,
           but we always use parens for consistency in serialization. */
        serialize_map(obj, sb, indent, depth, compact);
        break;
    }
}

/*---------------------------------------------------------------------------*/

char* cco_serialize(const cco_object_t *obj) {
    return cco_serialize_pretty(obj, 0);
}

char* cco_serialize_pretty(const cco_object_t *obj, int indent) {
    sb_t sb;
    if (sb_init(&sb) != 0) return NULL;
    serialize_value(obj, &sb, indent, 0, indent == 0);
    return sb.data;
}

/*---------------------------------------------------------------------------*/

static FILE* open_for_write(const char *filename) {
    FILE *fp;
#ifdef _MSC_VER
    fopen_s(&fp, filename, "w");
#else
    fp = fopen(filename, "w");
#endif
    return fp;
}

int cco_save_file(const cco_object_t *obj, const char *filename) {
    return cco_save_file_pretty(obj, filename, 0);
}

int cco_save_file_pretty(const cco_object_t *obj, const char *filename, int indent) {
    FILE *fp = open_for_write(filename);
    if (!fp) return CCO_ERR("cco_save_file: cannot open '%s'", filename);
    int ret = cco_save_stream_pretty(obj, fp, indent);
    fclose(fp);
    return ret;
}

int cco_save_stream(const cco_object_t *obj, FILE *fp) {
    return cco_save_stream_pretty(obj, fp, 0);
}

int cco_save_stream_pretty(const cco_object_t *obj, FILE *fp, int indent) {
    char *s = cco_serialize_pretty(obj, indent);
    if (!s) return -1;
    int r = (fputs(s, fp) == EOF) ? -1 : 0;
    free(s);
    return r;
}

/*---------------------------------------------------------------------------
 * Full serialization (Phase 7)
 *-------------------------------------------------------------------------*/
static void serialize_type_expr(const cco_type_expr_t *te, sb_t *sb);

static void serialize_type_primitive(cco_primitive_type_t p, sb_t *sb) {
    switch (p) {
    case CCO_PRIM_STRING:  sb_append_str(sb, "String"); break;
    case CCO_PRIM_INTEGER: sb_append_str(sb, "Integer"); break;
    case CCO_PRIM_FLOAT:   sb_append_str(sb, "Float"); break;
    case CCO_PRIM_BOOLEAN: sb_append_str(sb, "Boolean"); break;
    case CCO_PRIM_NONE:    sb_append_str(sb, "None"); break;
    }
}

static void serialize_type_expr(const cco_type_expr_t *te, sb_t *sb) {
    if (!te) { sb_append_str(sb, "None"); return; }
    switch (te->kind) {
    case CCO_TYPEKIND_PRIMITIVE:
        serialize_type_primitive(te->data.primitive, sb);
        break;
    case CCO_TYPEKIND_ARRAY:
        sb_append_str(sb, "Array<");
        serialize_type_expr(te->data.array.element_type, sb);
        sb_append_str(sb, ">");
        break;
    case CCO_TYPEKIND_OBJECT:
        sb_append_str(sb, "(");
        for (size_t i = 0; i < te->data.object.count; i++) {
            if (i > 0) sb_append_str(sb, ", ");
            sb_append_str(sb, te->data.object.fields[i].name);
            sb_append_str(sb, ": ");
            serialize_type_expr(te->data.object.fields[i].type, sb);
        }
        sb_append_str(sb, ")");
        break;
    case CCO_TYPEKIND_NAMED:
        sb_append_str(sb, te->data.named.name);
        break;
    case CCO_TYPEKIND_DYN:
        sb_append_str(sb, "dyn ");
        sb_append_str(sb, te->data.dyn.name);
        break;
    }
}

static void serialize_template_field(const cco_template_t *tmpl, size_t idx,
                                      sb_t *sb, int indent, int depth) {
    const char *name = cco_template_field_name(tmpl, idx);
    if (!name) return;
    for (int j = 0; j < (depth + 1) * indent; j++) sb_append_c(sb, ' ');
    sb_append_str(sb, name);
    sb_append_str(sb, ": Integer"); /* simplified type output */
    cco_expr_t *def = cco_template_field_default(tmpl, idx);
    if (def) sb_append_str(sb, " = ..."); /* expression serialization TBD */
}

char* cco_serialize_full(const cco_parse_result_t *res) {
    return cco_serialize_full_pretty(res, 0);
}

char* cco_serialize_full_pretty(const cco_parse_result_t *res, int indent) {
    if (!res) { CCO_ERR("cco_serialize_full: NULL result"); return NULL; }

    sb_t sb;
    if (sb_init(&sb) != 0) return NULL;

    cco_symbol_table_t *st = cco_parse_result_symbols((cco_parse_result_t*)res);
    cco_object_t *root = cco_parse_result_root((cco_parse_result_t*)res);

    size_t n = st ? cco_symtab_get_count(st) : 0;
    int first = 1;

    for (size_t i = 0; i < n; i++) {
        const char *name = cco_symtab_get_name(st, i);
        if (!name) continue;

        if (!first) { sb_append_str(&sb, ",\n"); }
        first = 0;

        /* Try typedef */
        cco_type_expr_t *td = cco_symtab_get_typedef(st, name);
        if (td) {
            sb_append_str(&sb, "$typedef.");
            sb_append_str(&sb, name);
            sb_append_str(&sb, ": ");
            serialize_type_expr(td, &sb);
            continue;
        }

        /* Try enum */
        const char **values;
        size_t vcount;
        if (cco_symtab_get_enum_values(st, name, &values, &vcount)) {
            sb_append_str(&sb, "$enum.");
            sb_append_str(&sb, name);
            sb_append_str(&sb, " = (");
            for (size_t j = 0; j < vcount; j++) {
                if (j > 0) sb_append_str(&sb, ", ");
                sb_append_str(&sb, values[j]);
            }
            sb_append_str(&sb, ")");
            continue;
        }

        /* Try template */
        cco_template_t *tmpl = cco_symtab_get_template(st, name);
        if (tmpl) {
            sb_append_str(&sb, "$temp.");
            sb_append_str(&sb, name);
            const char *parent = cco_template_get_parent_name(tmpl);
            if (parent) {
                sb_append_str(&sb, " + ");
                sb_append_str(&sb, parent);
            }
            sb_append_str(&sb, ": (");
            size_t fc = cco_template_field_count(tmpl);
            if (fc > 0) {
                if (indent) sb_append_str(&sb, "\n");
                for (size_t j = 0; j < fc; j++) {
                    if (j > 0) sb_append_str(&sb, indent ? ",\n" : ", ");
                    serialize_template_field(tmpl, j, &sb, indent, 0);
                }
                if (indent) sb_append_c(&sb, '\n');
            }
            sb_append_str(&sb, ")");
        }
    }

    /* Output root value */
    if (!first) sb_append_str(&sb, ",\n");
    if (root) {
        serialize_value(root, &sb, indent, 0, indent == 0);
    } else {
        sb_append_str(&sb, "None");
    }

    return sb.data;
}
