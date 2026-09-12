#include "diag.h"

#include <string.h>

/* Thread-local storage for diagnostics */
static _Thread_local cco_diag_entry_t t_diags[CCO_DIAG_MAX_COUNT];
static _Thread_local size_t t_diag_count = 0;
static _Thread_local cco_error_t t_first_error = CCO_OK;

void cco_diag_clear(void)
{
    t_diag_count = 0;
    t_first_error = CCO_OK;
}

void cco_diag_record(cco_error_t code, size_t line, size_t col, const char* func, const char* msg)
{
    if (t_first_error == CCO_OK && code != CCO_OK) {
        t_first_error = code;
    }

    if (t_diag_count >= CCO_DIAG_MAX_COUNT) {
        return; /* Drop if full */
    }

    cco_diag_entry_t* entry = &t_diags[t_diag_count++];
    entry->code = code;
    entry->line = line;
    entry->col = col;

    if (func) {
        strncpy(entry->func_name, func, sizeof(entry->func_name) - 1);
        entry->func_name[sizeof(entry->func_name) - 1] = '\0';
    } else {
        entry->func_name[0] = '\0';
    }

    if (msg) {
        strncpy(entry->message, msg, sizeof(entry->message) - 1);
        entry->message[sizeof(entry->message) - 1] = '\0';
    } else {
        entry->message[0] = '\0';
    }
}

const cco_diag_entry_t* cco_diag_get_all(size_t* count)
{
    if (count) {
        *count = t_diag_count;
    }
    return t_diags;
}

cco_error_t cco_diag_get_last_error(void)
{
    return t_first_error;
}
