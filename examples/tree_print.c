#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cnt/cco.h>

static void print_val(const cco_object_t* v, const char* pfx, int last);

static void print_map(const cco_object_t* v, const char* pfx)
{
    size_t count = cco_map_get_count(v);
    for (size_t i = 0; i < count; i++)
    {
        const char* key = cco_map_get_key(v, i);
        cco_object_t* val = cco_map_get_value(v, i);
        int is_last = (i == count - 1);
        printf("%s%s%s: ", pfx, is_last ? "+-- " : "|-- ", key);

        char np[256];
        snprintf(np, sizeof(np), "%s%s", pfx, is_last ? "    " : "|   ");
        print_val(val, np, is_last);
    }
}

static void print_arr(const cco_object_t* v, const char* pfx)
{
    size_t count = cco_array_get_count(v);
    for (size_t i = 0; i < count; i++)
    {
        cco_object_t* val = cco_array_get_item(v, i);
        int is_last = (i == count - 1);
        printf("%s%s[%zu]: ", pfx, is_last ? "+-- " : "|-- ", i);

        char np[256];
        snprintf(np, sizeof(np), "%s%s", pfx, is_last ? "    " : "|   ");
        print_val(val, np, is_last);
    }
}

static void print_escaped(const char* s, size_t len)
{
    printf("\"");
    for (size_t i = 0; i < len; i++)
    {
        switch (s[i])
        {
        case '\n':
            printf("\\n");
            break;
        case '\r':
            printf("\\r");
            break;
        case '\t':
            printf("\\t");
            break;
        case '\"':
            printf("\\\"");
            break;
        case '\\':
            printf("\\\\");
            break;
        default:
            putchar(s[i]);
            break;
        }
    }
    printf("\"");
}

static void print_val(const cco_object_t* v, const char* pfx, int last)
{
    (void)last;
    if (!v)
    {
        printf("None\n");
        return;
    }

    bool b;
    int64_t i;
    double f;
    const char* s;
    size_t slen;

    switch (cco_object_get_type(v))
    {
    case 0: /* NONE */
        printf("None\n");
        break;
    case 1: /* BOOLEAN */
        cco_object_get_boolean(v, &b);
        printf("%s\n", b ? "true" : "false");
        break;
    case 2: /* INTEGER */
        cco_object_get_integer(v, &i);
        printf("%lld\n", (long long)i);
        break;
    case 3: /* FLOAT */
        cco_object_get_float(v, &f);
        printf("%g\n", f);
        break;
    case 4: /* STRING */
        cco_object_get_string(v, &s, &slen);
        print_escaped(s, slen);
        printf("\n");
        break;
    case 5: /* ARRAY */
        printf("\n");
        print_arr(v, pfx);
        break;
    case 6: /* MAP */
        printf("\n");
        print_map(v, pfx);
        break;
    default:
        printf("Unknown\n");
        break;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: tree_print <file.cco>\n");
        return 1;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (!fp)
    {
        perror("fopen");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* source_buf = (char*)malloc((size_t)len + 1);
    if (!source_buf)
    {
        fclose(fp);
        return 1;
    }

    size_t r = fread(source_buf, 1, (size_t)len, fp);
    source_buf[r] = '\0';
    fclose(fp);

    cco_object_t* root = cco_parse_string(source_buf, r, NULL);
    if (!root)
    {
        cco_error_t err = cco_get_last_error();
        fprintf(stderr, "Parse error code: %d\n", err);
        free(source_buf);
        return 1;
    }

    printf(".\n");
    print_val(root, "    ", 1);

    cco_object_release(root);
    free(source_buf);
    return 0;
}
