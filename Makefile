LLVM_CXXFLAGS := $(shell llvm-config --cxxflags)
LLVM_LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core passes analysis transformutils)
CXX ?= clang++

.PHONY: all build test gpu clean
all: build

build:
	mkdir -p build
	$(CXX) -fPIC -shared src/OptimizationToolkit.cpp -o build/OptimizationToolkit.so $(LLVM_CXXFLAGS) $(LLVM_LDFLAGS)

test: build
	bash scripts/run_tests.sh

gpu: build
	bash scripts/run_gpu_experiment.sh

clean:
	rm -rf build
