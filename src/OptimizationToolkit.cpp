#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"

#include <cstdint>

using namespace llvm;

namespace {

struct Stats {
    uint64_t constantFolds = 0;
    uint64_t algebraicSimplifications = 0;
    uint64_t commonSubexpressions = 0;
    uint64_t deadInstructions = 0;

    bool changed() const {
        return constantFolds || algebraicSimplifications ||
               commonSubexpressions || deadInstructions;
    }
};

// Only apply identities whose correctness follows from ordinary LLVM IR
// semantics for these exact integer operations. Avoid x*0 and similar rules
// that can interact with poison/undef semantics.
static Value *simplifyIntegerBinaryOperator(BinaryOperator *BO) {
    if (!BO->getType()->isIntegerTy())
        return nullptr;

    Value *L = BO->getOperand(0);
    Value *R = BO->getOperand(1);
    auto *LC = dyn_cast<ConstantInt>(L);
    auto *RC = dyn_cast<ConstantInt>(R);

    switch (BO->getOpcode()) {
    case Instruction::Add:
        if (RC && RC->isZero()) return L;
        if (LC && LC->isZero()) return R;
        break;
    case Instruction::Sub:
        if (RC && RC->isZero()) return L;
        break;
    case Instruction::Mul:
        if (RC && RC->isOne()) return L;
        if (LC && LC->isOne()) return R;
        break;
    case Instruction::And:
        if (RC && RC->isMinusOne()) return L;
        if (LC && LC->isMinusOne()) return R;
        break;
    case Instruction::Or:
    case Instruction::Xor:
        if (RC && RC->isZero()) return L;
        if (LC && LC->isZero()) return R;
        break;
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr:
        if (RC && RC->isZero()) return L;
        break;
    default:
        break;
    }
    return nullptr;
}

// Local CSE within one basic block. Only considers binary operators, which
// are pure and have no memory side effects. isIdenticalTo checks opcode,
// operands, flags and other instruction properties.
static bool eliminateLocalCommonSubexpressions(BasicBlock &BB, Stats &S) {
    bool Changed = false;
    SmallVector<Instruction *, 16> Available;

    for (auto It = BB.begin(); It != BB.end();) {
        Instruction &I = *It++;
        auto *BO = dyn_cast<BinaryOperator>(&I);
        if (!BO)
            continue;

        Instruction *Existing = nullptr;
        for (Instruction *Candidate : Available) {
            if (Candidate->getParent() && Candidate->isIdenticalTo(BO)) {
                Existing = Candidate;
                break;
            }
        }

        if (Existing) {
            BO->replaceAllUsesWith(Existing);
            BO->eraseFromParent();
            ++S.commonSubexpressions;
            Changed = true;
        } else {
            Available.push_back(BO);
        }
    }
    return Changed;
}

class OptimizationToolkitPass
    : public PassInfoMixin<OptimizationToolkitPass> {
public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
        Stats S;
        bool ChangedInIteration;

        // Iterate because one transformation can expose another.
        do {
            ChangedInIteration = false;

            for (Function &F : M) {
                if (F.isDeclaration())
                    continue;

                for (BasicBlock &BB : F) {
                    for (auto It = BB.begin(); It != BB.end();) {
                        Instruction &I = *It++;

                        auto *BO = dyn_cast<BinaryOperator>(&I);
                        if (!BO)
                            continue;

                        if (Constant *C = ConstantFoldInstruction(
                                BO, M.getDataLayout())) {
                            BO->replaceAllUsesWith(C);
                            BO->eraseFromParent();
                            ++S.constantFolds;
                            ChangedInIteration = true;
                            continue;
                        }

                        if (Value *Replacement =
                                simplifyIntegerBinaryOperator(BO)) {
                            BO->replaceAllUsesWith(Replacement);
                            BO->eraseFromParent();
                            ++S.algebraicSimplifications;
                            ChangedInIteration = true;
                        }
                    }

                    if (eliminateLocalCommonSubexpressions(BB, S))
                        ChangedInIteration = true;

                    // Iterate safely while deleting trivially dead instructions.
                    for (auto It = BB.begin(); It != BB.end();) {
                        Instruction &I = *It++;
                        if (isInstructionTriviallyDead(&I)) {
                            I.eraseFromParent();
                            ++S.deadInstructions;
                            ChangedInIteration = true;
                        }
                    }
                }
            }
        } while (ChangedInIteration);

        errs() << "\n=== LLVM Optimization Toolkit ===\n"
               << "Constant folds: " << S.constantFolds << "\n"
               << "Algebraic simplifications: "
               << S.algebraicSimplifications << "\n"
               << "Local common subexpressions eliminated: "
               << S.commonSubexpressions << "\n"
               << "Dead instructions removed: " << S.deadInstructions << "\n"
               << "IR modified: " << (S.changed() ? "yes" : "no") << "\n"
               << "=================================\n";

        return S.changed()
            ? PreservedAnalyses::none()
            : PreservedAnalyses::all();
    }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "OptimizationToolkit",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "opt-toolkit") {
                        MPM.addPass(OptimizationToolkitPass());
                        return true;
                    }
                    return false;
                });
        }};
}
