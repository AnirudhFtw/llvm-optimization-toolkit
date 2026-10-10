# LLVM Optimization Toolkit

A C++ LLVM New Pass Manager plugin demonstrating a small, testable set of IR
optimizations, plus an experiment that sends LLVM IR through the NVPTX backend
and compares generated PTX.

## Implemented

- Constant folding using LLVM's `ConstantFoldInstruction` utility.
- Selected integer algebraic identities.
- Local common-subexpression elimination for identical binary operators in a
  single basic block.
- Dead-instruction elimination using LLVM's `isInstructionTriviallyDead`.
- Fixed-point iteration and transformation counters.
- IR verification and shell-based regression checks.
- GPU-targeted PTX generation experiment (requires LLVM built with NVPTX).

This is a learning/portfolio project, not a replacement for LLVM's production
optimization pipeline or NVIDIA's proprietary PTXAS.

## Requirements

- Linux or WSL Ubuntu
- LLVM/Clang development tools with matching major versions
- Bash, `llvm-config`, `opt`, and `llc`
- NVPTX backend for the GPU experiment

On Ubuntu, one possible starting point is:

```bash
sudo apt update
sudo apt install clang llvm-dev
```

Check versions and target availability:

```bash
llvm-config --version
opt --version
llc --version
```

The `opt` and `llvm-config` major versions must match. Check `llc --version`
for a registered `nvptx`/`nvptx64` target before running the GPU experiment.

## Build

```bash
chmod +x scripts/*.sh
./scripts/build.sh
```

Or use:

```bash
make build
```

## Run tests

```bash
./scripts/run_tests.sh
```

The tests check constant folding, algebraic simplification, local CSE, dead
instruction removal, and LLVM IR validity. The test suite is intentionally
small and should be expanded with edge cases before treating this as a mature
compiler pass.

## Run the GPU experiment

```bash
./scripts/run_gpu_experiment.sh
```

The script generates PTX from the original IR and the IR after our pass. It
also saves a diff. The backend may independently perform an optimization, so
identical PTX does not imply the custom pass failed; inspect the intermediate
IR and compare against LLVM's built-in optimization pipeline.

This experiment generates PTX; it does not execute the kernel on a GPU or
prove a runtime speedup. GPU execution and performance benchmarking are
separate future work.

## Project layout

```text
src/OptimizationToolkit.cpp
tests/basic.ll
tests/control_flow.ll
examples/gpu_kernel.ll
scripts/build.sh
scripts/run_tests.sh
scripts/run_gpu_experiment.sh
```

## Important limitations / future work

- CSE is local to a basic block; it is not global value numbering.
- Algebraic simplification supports only a small set of identities.
- The `control_flow.ll` file is a future test input for adding a control-flow
  pass; the custom plugin does not yet implement CFG simplification.
- The project does not implement register allocation, instruction scheduling,
  PTX assembly, or a complete source-language compiler.
- Add tests for integer edge cases, flags, multiple uses, branches, and
  interactions with LLVM's built-in passes.
- Benchmark with representative workloads before making performance claims.
