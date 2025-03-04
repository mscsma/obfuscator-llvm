//===- SplitBasicBlock.cpp - SplitBasicBlokc Obfuscation pass--------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the split basic block pass
//
//===----------------------------------------------------------------------===//

#include "CryptoUtils.h"
#include "SplitBasicBlocks.h"
#include "Utils.h"

#define DEBUG_TYPE "split"

using namespace llvm;

// Stats
STATISTIC(Split, "Basicblock splitted");

const int defaultObfRate = 30, defaultObfTime = 2;

static cl::opt<bool> SplitFlag("split", cl::init(false),
                           cl::desc("Enable basic block splitting"));

static cl::opt<int> SplitNum("split_num", cl::init(defaultObfTime),
                             cl::desc("Split <split_num> time each BB"));

static cl::opt<int>
    ObfProbRate("split_prob",
                cl::desc("Choose the probability [%] each basic blocks will be "
                         "split by the -split pass"),
                cl::value_desc("probability rate"), cl::init(defaultObfRate),
                cl::Optional);

namespace {
struct SplitBasicBlock : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  SplitBasicBlock() : FunctionPass(ID) {}

  bool runOnFunction(Function &F);
  void split(Function *f);

  bool containsPHI(BasicBlock *b);
  void shuffle(std::vector<int> &vec);
};
} // namespace

char SplitBasicBlock::ID = 0;
static RegisterPass<SplitBasicBlock> X("splitbbl", "BasicBlock splitting");

Pass *llvm::createSplitBasicBlock() {
  return new SplitBasicBlock();
}

PreservedAnalyses SplitBasicBlockPass::run(Function &F, FunctionAnalysisManager &AM){
  SplitBasicBlock split;
  split.runOnFunction(F);
  errs() << "asdfadsfdsamfdsadaks";
  return PreservedAnalyses::none();
}

bool SplitBasicBlock::runOnFunction(Function &F) {
  Function *tmp = &F;

  // Check if the number of applications is correct
  auto probs = readAnnotate(&F, ObfProbRate.ArgStr);
  if (!probs.empty()) {
    int value = ObfProbRate;
    if (!probs.getAsInteger(0, value))
      ObfProbRate.setValue(value);
  }
  auto nums = readAnnotate(tmp, SplitNum.ArgStr);
  if (!nums.empty()) {
    int value = SplitNum;
    if (!nums.getAsInteger(0, value))
      SplitNum.setValue(value);
  }
  if (!((SplitNum >= 1) && (SplitNum <= 10))) {
    errs() << "Split application basic block percentage\
            -split_num=x must be 1 < x <= 10";
    return false;
  }

  // Check if the number of applications is correct
  if (!((ObfProbRate > 0) && (ObfProbRate <= 100))) {
    errs() << "Split application basic blocks percentage "
              "-split_prob=x must be 0 < x <= 100";
    return false;
  }

  // Do we obfuscate
  if (toObfuscate(SplitFlag, tmp, "split")) {
    split(tmp);
    ++Split;
  }

  return false;
}

void SplitBasicBlock::split(Function *f) {
  std::vector<BasicBlock *> origBB;
  int splitN = SplitNum;

  // Save all basic blocks
  for (Function::iterator I = f->begin(), IE = f->end(); I != IE; ++I) {
    origBB.push_back(&*I);
  }

  for (std::vector<BasicBlock *>::iterator I = origBB.begin(),
                                           IE = origBB.end();
       I != IE; ++I) {
    
    if((int)llvm::cryptoutils->get_range(100) > ObfProbRate) {
      continue;
    }
    
    BasicBlock *curr = *I;

    // No need to split a 1 inst bb
    // Or ones containing a PHI node
    if (curr->size() < 2 || containsPHI(curr)) {
      continue;
    }

    // Check splitN and current BB size
    if ((size_t)splitN > curr->size()) {
      splitN = curr->size() - 1;
    }

    // Generate splits point
    std::vector<int> test;
    for (unsigned i = 1; i < curr->size(); ++i) {
      test.push_back(i);
    }

    // Shuffle
    if (test.size() != 1) {
      shuffle(test);
      std::sort(test.begin(), test.begin() + splitN);
    }

    // Split
    BasicBlock::iterator it = curr->begin();
    BasicBlock *toSplit = curr;
    int last = 0;
    for (int i = 0; i < splitN; ++i) {
      for (int j = 0; j < test[i] - last; ++j) {
        ++it;
      }
      last = test[i];
      if (toSplit->size() < 2)
        continue;
      toSplit = toSplit->splitBasicBlock(it, toSplit->getName() + ".split");
    }

    ++Split;
  }
}

bool SplitBasicBlock::containsPHI(BasicBlock *b) {
  for (BasicBlock::iterator I = b->begin(), IE = b->end(); I != IE; ++I) {
    if (isa<PHINode>(I)) {
      return true;
    }
  }
  return false;
}

void SplitBasicBlock::shuffle(std::vector<int> &vec) {
  int n = vec.size();
  for (int i = n - 1; i > 0; --i) {
    std::swap(vec[i], vec[cryptoutils->get_uint32_t() % (i + 1)]);
  }
}
