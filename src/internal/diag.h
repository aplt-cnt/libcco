#ifndef CNT_CCO_INTERNAL_DIAG_H
#define CNT_CCO_INTERNAL_DIAG_H

#include <stddef.h>

#include <cnt/cco_error.h>

#define CCO_DIAG_MSG_MAX 128
#define CCO_DIAG_MAX_COUNT 64

typedef struct
{
    cco_error_t code;
    size_t line;
    size_t col;
    char func_name[32];
    char message[CCO_DIAG_MSG_MAX];
} cco_diag_entry_t;

/* Clears the thread-local diagnostic storage */
void cco_diag_clear(void);

/* Records a diagnostic message. If storage is full (64 entries), it is dropped,
   but the first error code is preserved. Does NOT record sensitive data. */
void cco_diag_record(cco_error_t code, size_t line, size_t col,
                     const char* func, const char* msg);

/* Retrieves the array of recorded diagnostics and the count */
const cco_diag_entry_t* cco_diag_get_all(size_t* count);

/* Retrieves the first recorded error code */
cco_error_t cco_diag_get_last_error(void);

#endif /* CNT_CCO_INTERNAL_DIAG_H */
