#ifndef CNT_CCO_H
#define CNT_CCO_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Include error codes and options */
#include <cnt/cco_error.h>
#include <cnt/cco_options.h>

/* Opaque handles for internal types */
typedef struct cco_object_s cco_object_t;
typedef struct cco_parser_context_s cco_parser_context_t;
typedef struct cco_arena_s cco_arena_t;

/* --- Lifecycle APIs --- */

/* Parses a CCO document from a string. 
   Caller owns the returned object and MUST call cco_object_release() on it when done.
   If an error occurs, returns NULL and the specific error code can be fetched via diagnostic APIs. */
cco_object_t* cco_parse_string(const char* src, size_t len, const cco_parse_options_t* opts);

/* Serializes a CCO object into a newly allocated C-string. 
   Caller MUST free() the returned string. Returns NULL on failure. */
char* cco_serialize_to_string(const cco_object_t* obj, bool pretty);

/* --- Object Model APIs --- */

/* Retrieves the type of the given object. */
int cco_object_get_type(const cco_object_t* obj);

/* Increments the reference count of the object. */
void cco_object_retain(cco_object_t* obj);

/* Decrements the reference count of the object. Frees memory if count reaches zero. */
void cco_object_release(cco_object_t* obj);

/* --- Diagnostic APIs --- */

/* Retrieves the first error encountered during the last parse operation on this thread. */
cco_error_t cco_get_last_error(void);

/* Clears thread-local diagnostics. */
void cco_clear_diagnostics(void);

#ifdef __cplusplus
}
#endif

#endif /* CNT_CCO_H */
