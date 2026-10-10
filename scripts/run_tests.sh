#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
PLUGIN="./build/OptimizationToolkit.so"
if [[ ! -f "$PLUGIN" ]]; then
  echo "Plugin not found; run ./scripts/build.sh first" >&2
  exit 1
fi

mkdir -p build/test-results
opt -load-pass-plugin="$PLUGIN" -passes=opt-toolkit \
  -S tests/basic.ll -o build/test-results/basic.optimized.ll

opt -passes=verify -disable-output build/test-results/basic.optimized.ll

grep -Eq 'ret i32 42' build/test-results/basic.optimized.ll
grep -Eq 'ret i32 %x' build/test-results/basic.optimized.ll
if grep -Eq '%unused = ' build/test-results/basic.optimized.ll; then
  echo "FAIL: unused instruction remains" >&2
  exit 1
fi

# Check that duplicate add was removed: only one add of %x and %y remains.
COUNT="$(grep -Ec 'add i32 %x, %y' build/test-results/basic.optimized.ll || true)"
if [[ "$COUNT" -gt 1 ]]; then
  echo "FAIL: local CSE did not eliminate duplicate add" >&2
  exit 1
fi

echo "PASS: basic optimization tests"
echo "Optimized IR: build/test-results/basic.optimized.ll"
