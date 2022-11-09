#ifndef _OBFUSCATION_DATA_SCRAMBLING_H_
#define _OBFUSCATION_DATA_SCRAMBLING_H_

#include "llvm/Pass.h"
#include <llvm/IR/PassManager.h>

#include <cstdint>

namespace llvm {
Pass *createDataScrambling();
class DataScramblingPass : public PassInfoMixin<DataScramblingPass> { // New PassManager
public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

    static bool isRequired() { return true; }
};
}

#endif