#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
CXX="${CXX:-clang++}"
OUT="${OUT:-build/OptimizationToolkit.so}"
mkdir -p build
"$CXX" -fPIC -shared src/OptimizationToolkit.cpp -o "$OUT" \
  $(llvm-config --cxxflags --ldflags --system-libs --libs core passes analysis transformutils)
echo "Built: $OUT"
