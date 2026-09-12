#!/usr/bin/env bash
set -euo pipefail
cd include
for h in cnt/*.h; do
    printf '#include <%s>\n' "$h" | cc -x c -std=c11 -I. -fsyntax-only -
    printf '  ok: %s\n' "$h"
done
cd ../include
for h in cnt/*.hpp; do
    printf '#include <%s>\n' "$h" | c++ -x c++ -std=c++11 -I. -fsyntax-only -
    printf '  ok: %s\n' "$h"
done