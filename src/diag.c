#include <cnt/cco_p.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define CCO_ERR_BUF_SIZE 4096

/* Thread-local error string (legacy) */
static char g_error_buf[CCO_ERR_BUF_SIZE];

/* Structured diagnostics */
static cco_diag_t  g_diags[CCO_MAX_DIAGS];
static int         g_diag_count;
static const char *g_diag_source;

void cco_diag_clear(void) {
    g_error_buf[0] = '\0';
    g_diag_count = 0;
}

void cco_diag_set_source(const char *source) {
    g_diag_source = source;
}

const char* cco_diag_get_source(void) {
    return g_diag_source;
}

static int diag_add(const char *file, size_t line, size_t col,
                    const char *msg, int is_error) {
    if (g_diag_count < CCO_MAX_DIAGS) {
        cco_diag_t *d = &g_diags[g_diag_count++];
        d->file    = file ? file : "<input>";
        d->line    = line;
        d->col     = col;
        d->message = msg;
        d->is_error = is_error;
    }
    /* Always update last-error string */
    if (is_error) {
        snprintf(g_error_buf, CCO_ERR_BUF_SIZE, "%s:%zu:%zu: %s",
                 file ? file : "<input>", line, col, msg);
    }
    return is_error ? -1 : 0;
}

int cco_diag_vfail(const char *file, size_t line, size_t col,
                   const char *fmt, ...) {
    char buf[CCO_ERR_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, CCO_ERR_BUF_SIZE, fmt, ap);
    va_end(ap);
    return diag_add(file, line, col, buf, 1);
}

int cco_diag_vwarn(const char *file, size_t line, size_t col,
                   const char *fmt, ...) {
    char buf[CCO_ERR_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, CCO_ERR_BUF_SIZE, fmt, ap);
    va_end(ap);
    return diag_add(file, line, col, buf, 0);
}

int cco_diag_count(void) {
    return g_diag_count;
}

const cco_diag_t* cco_diag_get(int i) {
    if (i < 0 || i >= g_diag_count) return NULL;
    return &g_diags[i];
}

void cco_diag_print(const cco_diag_t *d, const char *source) {
    if (!d) return;
    fprintf(stderr, "%s:%zu:%zu: %s%s\n",
            d->file, d->line, d->col,
            d->is_error ? "error: " : "warning: ",
            d->message);

    if (!source) return;

    /* Find the source line containing the error */
    const char *line_start = source;
    const char *p = source;
    size_t current_line = 1;

    while (*p && current_line < d->line) {
        if (*p == '\n') {
            current_line++;
            line_start = p + 1;
        }
        p++;
    }
    if (!*p) return;

    /* Find end of this line */
    const char *line_end = line_start;
    while (*line_end && *line_end != '\n') line_end++;

    /* Print line number and content */
    fprintf(stderr, "   |\n");
    fprintf(stderr, "%3zu | %.*s\n", d->line, (int)(line_end - line_start), line_start);

    /* Print caret marker */
    fprintf(stderr, "   | ");
    size_t caret_col = d->col > 0 ? d->col - 1 : 0;
    for (size_t i = 0; i < caret_col; i++) {
        /* Handle tabs */
        if (line_start + i < line_end && line_start[i] == '\t')
            fputc('\t', stderr);
        else
            fputc(' ', stderr);
    }
    fprintf(stderr, "^~~\n");
}

void cco_diag_print_all(const char *source) {
    for (int i = 0; i < g_diag_count; i++) {
        cco_diag_print(&g_diags[i], source);
    }
}

const char* cco_last_error(void) {
    return g_error_buf[0] ? g_error_buf : "No error";
}

void cco_clear_error(void) {
    cco_diag_clear();
}
