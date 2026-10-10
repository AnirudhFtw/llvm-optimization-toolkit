#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
PLUGIN="./build/OptimizationToolkit.so"
mkdir -p build/gpu-results

if ! llc --version | grep -qi 'nvptx'; then
  echo "This LLVM llc build may not include the NVPTX target." >&2
  echo "Check: llc --version" >&2
  exit 2
fi

llc -O0 -march=nvptx64 -mcpu=sm_70 \
  examples/gpu_kernel.ll -o build/gpu-results/before.ptx

opt -load-pass-plugin="$PLUGIN" -passes=opt-toolkit \
  -S examples/gpu_kernel.ll -o build/gpu-results/gpu_optimized.ll

llc -O0 -march=nvptx64 -mcpu=sm_70 \
  build/gpu-results/gpu_optimized.ll -o build/gpu-results/after.ptx

diff -u build/gpu-results/before.ptx build/gpu-results/after.ptx \
  > build/gpu-results/ptx.diff || true

echo "Generated:"
echo "  build/gpu-results/before.ptx"
echo "  build/gpu-results/after.ptx"
echo "  build/gpu-results/ptx.diff"
echo "Note: identical PTX is possible if backend code generation independently performs the same simplification."
