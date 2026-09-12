#!/usr/bin/env bash
set -euo pipefail

# Cross-check that every CCO_ENABLE_* macro mentioned in the docs also
# appears somewhere in src/ or include/. The regex requires at least one
# uppercase letter after the trailing underscore, so a literal
# "CCO_ENABLE_*" in prose (glob-style) does not produce a false positive.

docs=$(grep -ohE 'CCO_ENABLE_[A-Z][A-Z0-9_]*' docs/feature-flags.md | sort -u || true)
src=$(grep -rhoE 'CCO_ENABLE_[A-Z][A-Z0-9_]*' src/ include/ 2>/dev/null | sort -u || true)

if [ -z "$docs" ]; then
    echo "ERROR: no CCO_ENABLE_* macros found in docs/feature-flags.md" >&2
    exit 1
fi

if [ -z "$src" ]; then
    echo "ERROR: no CCO_ENABLE_* macros found in src/ or include/" >&2
    exit 1
fi

if ! diff <(echo "$docs") <(echo "$src") > /dev/null; then
    echo "ERROR: CCO_ENABLE_* macros differ between docs and source:" >&2
    diff <(echo "$docs") <(echo "$src") >&2 || true
    exit 1
fi

echo "OK: CCO_ENABLE_* macro set matches between docs/feature-flags.md and src/+include/"
