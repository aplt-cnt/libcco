#ifndef CNT_CCO_H
#define CNT_CCO_H

/*
 * libcco public C API.
 *
 * Status: indev. The complete public surface is filled in as
 * implementation lands. docs/api.md is the authoritative reference and
 * MUST be updated in the same commit as any public symbol change.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ---------------------------------------------------------------- *
     * Compile-time feature flags.
     *
     * Each macro below is a build-time switch. When a flag is OFF, the
     * corresponding syntax is rejected at parse time with
     * CCO_ERR_FORBIDDEN, and any public API for that feature returns
     * CCO_ERR_FORBIDDEN instead of silently degrading.
     *
     * The caller's translation units and the library MUST be compiled
     * with the same flag set. Mixing them produces different
     * cco_parse_options_t layouts and is an ABI mismatch. See
     * docs/feature-flags.md for the full matrix.
     * ---------------------------------------------------------------- */

#ifndef CCO_ENABLE_FORMAT
#define CCO_ENABLE_FORMAT 0
#endif

#ifndef CCO_ENABLE_EVAL
#define CCO_ENABLE_EVAL 0
#endif

#ifndef CCO_ENABLE_COMMENT_PRESERVE
#define CCO_ENABLE_COMMENT_PRESERVE 0
#endif

#ifndef CCO_ENABLE_ERROR_RECOVERY
#define CCO_ENABLE_ERROR_RECOVERY 0
#endif

#ifndef CCO_ENABLE_ENV
#define CCO_ENABLE_ENV 0
#endif

#ifndef CCO_ENABLE_STATIC_CALLS
#define CCO_ENABLE_STATIC_CALLS 1
#endif

#ifndef CCO_ENABLE_CONSTRUCTORS
#define CCO_ENABLE_CONSTRUCTORS 1
#endif

    /* ---------------------------------------------------------------- *
     * Version
     * ---------------------------------------------------------------- */

#define CCO_VERSION_MAJOR 0
#define CCO_VERSION_MINOR 1
#define CCO_VERSION_PATCH 0

    /* ---------------------------------------------------------------- *
     * Opaque handle types. Bodies are private to src/.
     * ---------------------------------------------------------------- */

    typedef struct cco_object_s cco_object_t;
    typedef struct cco_array_s cco_array_t;
    typedef struct cco_parse_result_s cco_parse_result_t;
    typedef struct cco_symbol_table_s cco_symbol_table_t;
    typedef struct cco_template_s cco_template_t;
    typedef struct cco_expr_s cco_expr_t;

    /* ---------------------------------------------------------------- *
     * Error codes.
     * ---------------------------------------------------------------- */

    typedef enum
    {
        CCO_OK = 0,
        CCO_ERR_PARSE = -1,
        CCO_ERR_NOT_FOUND = -2,
        CCO_ERR_TYPE_MISMATCH = -3,
        CCO_ERR_OUT_OF_RANGE = -4,
        CCO_ERR_IO = -5,
        CCO_ERR_FORBIDDEN = -6,
        CCO_ERR_NOMEM = -7,
        CCO_ERR_DEPTH_EXCEEDED = -8,
        CCO_ERR_INVALID_ARG = -9
    } cco_err_t;

    /*
     * Function declarations are added here as implementation lands. Every
     * addition MUST also update docs/api.md.
     */

#ifdef __cplusplus
}
#endif

#endif /* CNT_CCO_H */
