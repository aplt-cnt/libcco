/**
 * template_instantiation.c — Declare templates, instantiate them.
 *
 * Compile: cc -I../include template_instantiation.c -L../target/Debug -lcco
 */
#include <cnt/cco.h>
#include <stdio.h>

int main(void) {
    const char *src =
        "$temp.Point: ( x: Integer, y: Integer ),\n"
        "$temp.Circle + Point: ( radius: Float ),\n"
        "shapes: (\n"
        "    origin: #Point(0, 0),\n"
        "    unit:   #Point(1, 1),\n"
        "    c:      #Circle(5, 5, 2.5),\n"
        "    nc:     #Circle(.x: 10, .y: 20, .radius: 3.0)\n"
        ")\n";

    cco_parse_result_t *res = cco_parse_full(src);
    if (!res) {
        fprintf(stderr, "Error: %s\n", cco_last_error());
        return 1;
    }

    cco_object_t *shapes = cco_object_get(cco_parse_result_root(res), "shapes");
    if (!shapes) { fprintf(stderr, "no shapes\n"); return 1; }

    cco_object_t *origin = cco_object_get(shapes, "origin");
    printf("origin = %s", cco_object_template_name(origin));
    printf(" (%lld, %lld)\n",
           cco_int_get(cco_object_get(origin, "x")),
           cco_int_get(cco_object_get(origin, "y")));

    cco_object_t *circle = cco_object_get(shapes, "c");
    printf("c      = %s", cco_object_template_name(circle));
    printf(" (%lld, %lld, %.1f)\n",
           cco_int_get(cco_object_get(circle, "x")),
           cco_int_get(cco_object_get(circle, "y")),
           cco_float_get(cco_object_get(circle, "radius")));

    cco_object_t *nc = cco_object_get(shapes, "nc");
    printf("nc     = %s", cco_object_template_name(nc));
    printf(" (%lld, %lld, %.1f)\n",
           cco_int_get(cco_object_get(nc, "x")),
           cco_int_get(cco_object_get(nc, "y")),
           cco_float_get(cco_object_get(nc, "radius")));

    cco_parse_result_free(res);
    return 0;
}
