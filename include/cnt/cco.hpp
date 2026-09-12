#ifndef CNT_CCO_HPP
#define CNT_CCO_HPP

/*
 * libcco C++ RAII wrapper.
 *
 * Status: indev. The wrapper lands once the C surface is stable
 * enough to build on. See docs/api.md for the reference.
 */

#include <cnt/cco.h>

namespace cnt
{

/*
 * The RAII layer will live in this namespace: cnt::Value, cnt::Array,
 * cnt::parse, cnt::ParseResult, cnt::ValueProxy, cnt::ArrayProxy.
 * Symbols are added as implementation lands.
 */

}  // namespace cnt

#endif  // CNT_CCO_HPP
