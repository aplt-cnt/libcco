#!/bin/bash
# Find legacy | used for coalesce instead of ??
grep -RnE '\$\(.*\|.*\)' . | grep -v '||'
