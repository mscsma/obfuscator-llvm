#include "DataScrambling.h"
#include "Utils.h"
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Pass.h>
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#include <llvm/Support/Casting.h>
#include <llvm/Transforms/Utils/LoopUtils.h>
#include <string>
#include <cstdint>

#define DEBUG_TYPE "split"

using namespace llvm;

static cl::opt<bool> ScramblingFlag("data_scramble", cl::init(false),
                                    cl::desc("Enable data scrambling"));

namespace {
    struct DataScrambling : public ModulePass {
        static char ID; // Pass identification, replacement for typeid

        DataScrambling() : ModulePass(ID) {}

        bool runOnModule(Module &M) override;
        static void encrypt(const char *in, size_t size, char *out);
        static void encrypt(const intmax_t &in, intmax_t &out);
    };
}

char DataScrambling::ID = 0;
static RegisterPass<DataScrambling> X("data_scramble", "Data scrambling");

Pass *llvm::createDataScrambling() {
    return new DataScrambling();
}

PreservedAnalyses DataScramblingPass::run(Module &M, ModuleAnalysisManager &AM) {
    DataScrambling pass;
    errs() << "SADCKSADNFJKADSNFKA";
    pass.runOnModule(M);
    return PreservedAnalyses::none();
}


bool DataScrambling::runOnModule(Module &M) {
    /*for (auto &Global : M.getGlobalList()) {
        if (!Global.hasInitializer())
            continue;
        //Global.getInitializer()->dump();
        // errs() << Global.getName() << "\n";
        if (isa<ConstantAggregate> (Global.getInitializer())) {
            auto cda = cast<ConstantAggregate>(Global.getInitializer());
            // cda->getRawDataValues();
            cda->setOperand(0, ConstantDataVector::get(M.getContext(), std::vector<uint>({4,5,6})));
            auto *tmp = cda->getOperand((unsigned)0);
            errs() << "ConstArray ";
            
            if (isa<ConstantDataSequential>(tmp)) {
                auto *tmp2 = cast<ConstantDataSequential>(tmp);
                errs() << Global.getName() << " " <<  tmp2->getElementAsInteger(1) << "\n";
            }
        }
        if (isa<ConstantDataSequential> (Global.getInitializer())) {
            auto cda = cast<ConstantDataSequential>(Global.getInitializer());
            // cda->getRawDataValues();
            errs() << "ConstDataArray ";
            errs() << Global.getName() << " " << cda->getRawDataValues() << "\n";
            errs() << Global.getName() << " " << cda->getNumElements() * cda->getElementByteSize() << "\n";
            errs() << Global.getName() << " " << cda->getElementAsInteger(0) << "\n";
        }
        else if (isa<ConstantData> (Global.getInitializer())) {
            errs() << "ConstData ";
            auto cd = cast<ConstantData> (Global.getInitializer());
            const APInt v;
            errs() << Global.getName() << " " << cd->getIntegerValue(Global.getInitializer()->getType(), v) << "\n";
        }
    }*/
    std::vector<GlobalVariable *> toDelete;

    for (auto &g : M.globals()) {
        auto *global = dyn_cast<GlobalVariable>(&g);
        if (!global)
            continue;
        if (!global->hasInitializer())
            continue;
        //errs() << global->getName() << "\n";

        const char *data;
        Constant * init = global->getInitializer();
        Constant * newInit = init;
        if (isa<ConstantDataSequential> (init)) {
            auto *cda = dyn_cast<ConstantDataSequential>(init);
            data = cda->getRawDataValues().data();
            size_t size = cda->getNumElements() * cda->getElementByteSize();
            //newInit = new ConstantDataSequential(cda->getType(), cda->getElementType(), data);
            char *newData = new char[size];
            DataScrambling::encrypt(data, size, newData);
            newInit = ConstantDataVector::getRaw(newData, cda->getNumElements(), cda->getElementType());
        } /*else if (isa<ConstantInt> (init)) {
            auto cd = cast<ConstantInt> (init);
            if (ConstantInt* CI = dyn_cast<ConstantInt>(cd)) {
                intmax_t newInt;
                DataScrambling::encrypt(CI->getSExtValue(), newInt);
                newInit = ConstantInt::get(CI->getType(), newInt);
                //errs() << dyn_cast<ConstantInt>(newInit)->getSExtValue() << "\n";
            }

        } */
        /*auto *newGlobal = new GlobalVariable(
                global->getValueType(),
                global->isConstant(),
                global->getLinkage(),
                newInit,
                "",
                global->getThreadLocalMode(),
                global->getAddressSpace(),
                global->isExternallyInitialized());
        global->replaceAllUsesWith(newGlobal);*/
        global->setInitializer(newInit);

        //global->replaceAllUsesWith(global);
        /*for (size_t i = 0; i < global->getNumUses(); ++i) {
            auto &Use = global->getOperandUse(i);
            errs() << Use->getName() << "\n";
            Use.set(global->getInitializer());
        }*/
        //toDelete.push_back(global);
        /*global->eraseFromParent();*/
    }

    /*for (auto &global : toDelete) {
        global->eraseFromParent();
    }*/
    return false;
} 

void DataScrambling::encrypt(const char *in, size_t size, char *out) {
    for (size_t i = 0; i < size; ++i) {
        out[i] = in[i] ^ 42;
    }
}

void DataScrambling::encrypt(const intmax_t &in, intmax_t &out) {
    out = in * in;
}