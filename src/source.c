#include <cnt/cco_p.h>
#include <string.h>

void cco_source_init(cco_source_t *s, const char *text, const char *filename) {
    s->data     = text;
    s->filename = filename ? filename : "<input>";
    s->len      = strlen(text);
}
