//===--- NodeCppTidyModule.cpp - clang-tidy -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "ArrayTypeCheck.h"
#include "NakedPtrFieldCheck.h"
#include "NakedPtrFuncCheck.h"
#include "NewArrayExprCheck.h"
#include "NewExprCheck.h"
#include "PtrArithmeticCheck.h"
#include "StaticStorageCheck.h"

namespace clang {
namespace tidy {
namespace nodecpp {

/// A module containing checks of the Node.C++ infrastructure
class NodeCppModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<ArrayTypeCheck>(
        "nodecpp-array-type");
    CheckFactories.registerCheck<NakedPtrFieldCheck>(
        "nodecpp-naked-ptr-field");
    CheckFactories.registerCheck<NakedPtrFuncCheck>(
        "nodecpp-naked-ptr-func");
    CheckFactories.registerCheck<NewArrayExprCheck>(
        "nodecpp-new-array-expr");
    CheckFactories.registerCheck<NewExprCheck>(
        "nodecpp-new-expr");
    CheckFactories.registerCheck<PtrArithmeticCheck>(
        "nodecpp-ptr-arithmetic");
    CheckFactories.registerCheck<StaticStorageCheck>(
        "nodecpp-static-storage");
  }
};

// Register the LLVMTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<NodeCppModule>
    X("nodecpp-module", "Adds checks for the Node.C++ infrastructure.");

} // namespace nodecpp

// This anchor is used to force the linker to link in the generated object file
// and thus register the NodeCppModule.
volatile int NodeCppModuleAnchorSource = 0;

} // namespace tidy
} // namespace clang