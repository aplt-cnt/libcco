#include <cnt/cco.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_escaped(const char *s) {
    putchar('"');
    for (; *s; s++) {
        switch (*s) {
        case '"':  printf("\\\""); break;
        case '\\': printf("\\\\"); break;
        case '\n': printf("\\n");  break;
        default:   putchar(*s);
        }
    }
    putchar('"');
}

static void print_val(cco_object_t *v, const char *pfx, int last, int top);

static void print_map(cco_object_t *v, const char *pfx, int last) {
    size_t n = cco_object_size(v);
    printf("(map) [%zu]\n", n);
    if (n == 0) return;
    cco_map_iter_t *it = cco_map_iter_new(v);
    const char *key; cco_object_t *val;
    size_t i = 0;
    while (cco_map_iter_next(it, &key, &val)) {
        int is_last = (++i == n);
        printf("%s%s%s: ", pfx, is_last ? "+-- " : "|-- ", key);
        char np[256];
        snprintf(np, sizeof(np), "%s%s", pfx, is_last ? "    " : "|   ");
        print_val(val, np, is_last, 0);
    }
    cco_map_iter_free(it);
}

static void print_arr(cco_object_t *v, const char *pfx, int last) {
    const cco_array_t *a = cco_object_as_array(v);
    size_t n = cco_array_size(a);
    printf("(array) [%zu]\n", n);
    for (size_t i = 0; i < n; i++) {
        int is_last = (i == n - 1);
        printf("%s%s", pfx, is_last ? "+-- " : "|-- ");
        char np[256];
        snprintf(np, sizeof(np), "%s%s", pfx, is_last ? "    " : "|   ");
        print_val(cco_array_get(a, i), np, is_last, 0);
    }
}

static void print_ti(cco_object_t *v, const char *pfx, int last) {
    size_t n = cco_object_size(v);
    printf("(%s) [%zu]\n", cco_object_template_name(v), n);
    cco_map_iter_t *it = cco_map_iter_new(v);
    const char *key; cco_object_t *val;
    size_t i = 0;
    while (cco_map_iter_next(it, &key, &val)) {
        int is_last = (++i == n);
        printf("%s%s%s: ", pfx, is_last ? "+-- " : "|-- ", key);
        char np[256];
        snprintf(np, sizeof(np), "%s%s", pfx, is_last ? "    " : "|   ");
        print_val(val, np, is_last, 0);
    }
    cco_map_iter_free(it);
}

static void print_val(cco_object_t *v, const char *pfx, int last, int top) {
    (void)last; (void)top;
    if (!v) { printf("None\n"); return; }
    switch (cco_object_type(v)) {
    case CCO_TYPE_NONE:    printf("None\n"); break;
    case CCO_TYPE_BOOL:    printf("%s\n", cco_bool_get(v) ? "true" : "false"); break;
    case CCO_TYPE_INT:     printf("%lld\n", cco_int_get(v)); break;
    case CCO_TYPE_FLOAT:   printf("%g\n", cco_float_get(v)); break;
    case CCO_TYPE_STRING:  print_escaped(cco_string_get(v)); printf("\n"); break;
    case CCO_TYPE_MAP:               print_map(v, pfx, 0); break;
    case CCO_TYPE_ARRAY:             print_arr(v, pfx, 0); break;
    case CCO_TYPE_TEMPLATE_INSTANCE: print_ti(v, pfx, 0); break;
    }
}

static void print_type(cco_type_expr_t *t) {
    switch (cco_type_expr_kind(t)) {
    case CCO_TYPEKIND_PRIMITIVE:
        switch (cco_type_expr_primitive(t)) {
        case CCO_PRIM_STRING:  printf("String"); break;
        case CCO_PRIM_INTEGER: printf("Integer"); break;
        case CCO_PRIM_FLOAT:   printf("Float"); break;
        case CCO_PRIM_BOOLEAN: printf("Boolean"); break;
        case CCO_PRIM_NONE:    printf("None"); break;
        }
        break;
    case CCO_TYPEKIND_NAMED: printf("%s", cco_type_expr_name(t)); break;
    case CCO_TYPEKIND_DYN:   printf("dyn %s", cco_type_expr_name(t)); break;
    case CCO_TYPEKIND_ARRAY: printf("Array"); break;
    case CCO_TYPEKIND_OBJECT: printf("object"); break;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: tree_print <file.cco>\n");
        return 1;
    }

    /* Read file content for source context */
    FILE *fp = fopen(argv[1], "rb");
    char *source_buf = NULL;
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        source_buf = (char*)malloc((size_t)len + 1);
        if (source_buf) {
            size_t r = fread(source_buf, 1, (size_t)len, fp);
            source_buf[r] = '\0';
        }
        fclose(fp);
    }

    cco_parse_result_t *res = cco_parse_full_from_file(argv[1]);
    if (!res) {
        cco_diag_print_all(source_buf);
        free(source_buf);
        return 1;
    }
    free(source_buf);

    cco_symbol_table_t *st = cco_parse_result_symbols(res);
    cco_object_t *root = cco_parse_result_root(res);

    printf(".\n");

    /* Declarations */
    size_t nd = st ? cco_symtab_get_count(st) : 0;
    for (size_t i = 0; i < nd; i++) {
        const char *name = cco_symtab_get_name(st, i);
        int last = (i == nd - 1) && !root;
        printf("%s", last ? "+-- " : "|-- ");

        cco_type_expr_t *td = cco_symtab_get_typedef(st, name);
        if (td) {
            printf("$typedef %s :: ", name);
            print_type(td);
            printf("\n");
            continue;
        }

        const char **vals;
        size_t vc;
        if (cco_symtab_get_enum_values(st, name, &vals, &vc)) {
            printf("$enum %s = ", name);
            for (size_t j = 0; j < vc; j++) {
                if (j > 0) printf(" | ");
                printf("%s", vals[j]);
            }
            printf("\n");
            continue;
        }

        cco_template_t *tmpl = cco_symtab_get_template(st, name);
        if (tmpl) {
            printf("$temp %s", name);
            const char *p = cco_template_get_parent_name(tmpl);
            if (p) printf(" + %s", p);
            printf("\n");
            size_t fc = cco_template_field_count(tmpl);
            for (size_t j = 0; j < fc; j++) {
                int fl = (j == fc - 1);
                printf("%s%s%s: Integer\n",
                       last ? "    " : "|   ",
                       fl   ? "+-- " : "|-- ",
                       cco_template_field_name(tmpl, j));
                (void)fl;
            }
            continue;
        }
    }

    /* Root value */
    if (root) {
        const char *pfx = nd > 0 ? "    " : "    ";
        printf("%s", nd > 0 ? "+-- " : "+-- ");
        print_val(root, pfx, 1, 1);
    }

    cco_parse_result_free(res);
    return 0;
}
