/**
 * parse_simple.c — Parse .cco text and inspect the result.
 *
 * Compile: cc -I../include parse_simple.c -L../target/Debug -lcco
 */
#include <cnt/cco.h>
#include <stdio.h>

int main(void) {
    const char *src =
        "( project: \"libcco\",\n"
        "  version: 1,\n"
        "  tags: ( \"c\", \"config\", \"serialization\" ),\n"
        "  enabled: true,\n"
        "  count: 42 )\n";

    cco_object_t *root = cco_parse(src);
    if (!root) {
        fprintf(stderr, "Parse error: %s\n", cco_last_error());
        return 1;
    }

    /* Read fields */
    printf("project  = %s\n", cco_string_get(cco_object_get(root, "project")));
    printf("version  = %lld\n", cco_int_get(cco_object_get(root, "version")));
    printf("enabled  = %s\n", cco_bool_get(cco_object_get(root, "enabled")) ? "true" : "false");
    printf("count    = %lld\n", cco_int_get(cco_object_get(root, "count")));

    /* Iterate array */
    cco_object_t *tags_obj = cco_object_get(root, "tags");
    const cco_array_t *tags = cco_object_as_array(tags_obj);
    printf("tags     = [");
    for (size_t i = 0; i < cco_array_size(tags); i++) {
        if (i > 0) printf(", ");
        printf("\"%s\"", cco_string_get(cco_array_get(tags, i)));
    }
    printf("]\n");

    /* Iterate map */
    printf("\nAll fields:\n");
    cco_map_iter_t *it = cco_map_iter_new(root);
    const char *key;
    cco_object_t *val;
    while (cco_map_iter_next(it, &key, &val)) {
        printf("  %s: ", key);
        if (cco_is_string(val))  printf("\"%s\"", cco_string_get(val));
        else if (cco_is_int(val))    printf("%lld", cco_int_get(val));
        else if (cco_is_bool(val))   printf("%s", cco_bool_get(val) ? "true" : "false");
        else if (cco_is_array(val))  printf("[%zu items]", cco_array_size(cco_object_as_array(val)));
        else if (cco_is_none(val))   printf("None");
        else printf("?");
        printf("\n");
    }
    cco_map_iter_free(it);

    cco_object_free(root);
    return 0;
}
