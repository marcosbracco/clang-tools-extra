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
#include "ConstCheck.h"
#include "MayExtendDeclCheck.h"
#include "MayExtendLambdaCheck.h"
#include "NakedAssignmentCheck.h"
#include "NakedPtrFuncCheck.h"
#include "NakedPtrReturnCheck.h"
#include "NakedStructCheck.h"
#include "NewExprCheck.h"
#include "NoCastCheck.h"
#include "PtrArithmeticCheck.h"
#include "RawPointerAssignmentCheck.h"
#include "RawPointerDereferenceCheck.h"
#include "StaticStorageCheck.h"
#include "StdFunctionCheck.h"
#include "UnionCheck.h"
#include "VarDeclCheck.h"

namespace clang {
namespace tidy {
namespace nodecpp {

/// A module containing checks of the Node.C++ infrastructure
class NodeCppModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<ArrayTypeCheck>(
        "nodecpp-array-type");
    CheckFactories.registerCheck<ConstCheck>(
        "nodecpp-const");
    CheckFactories.registerCheck<MayExtendDeclCheck>(
        "nodecpp-may-extend-decl");
    CheckFactories.registerCheck<MayExtendLambdaCheck>(
        "nodecpp-may-extend-lambda");
    CheckFactories.registerCheck<NakedAssignmentCheck>(
        "nodecpp-naked-assignment");
    CheckFactories.registerCheck<NakedPtrFuncCheck>(
        "nodecpp-naked-ptr-func");
    CheckFactories.registerCheck<NakedPtrReturnCheck>(
        "nodecpp-naked-ptr-return");
    CheckFactories.registerCheck<NakedStructCheck>(
        "nodecpp-naked-struct");
    CheckFactories.registerCheck<NewExprCheck>(
        "nodecpp-new-expr");
    CheckFactories.registerCheck<NoCastCheck>(
        "nodecpp-no-cast");
    CheckFactories.registerCheck<PtrArithmeticCheck>(
        "nodecpp-ptr-arithmetic");
    CheckFactories.registerCheck<RawPointerAssignmentCheck>(
        "nodecpp-raw-pointer-assignment");
    CheckFactories.registerCheck<RawPointerDereferenceCheck>(
        "nodecpp-raw-pointer-dereference");
    CheckFactories.registerCheck<StaticStorageCheck>(
        "nodecpp-static-storage");
    CheckFactories.registerCheck<StdFunctionCheck>(
        "nodecpp-std-function");
    CheckFactories.registerCheck<UnionCheck>(
        "nodecpp-union");
    CheckFactories.registerCheck<VarDeclCheck>(
        "nodecpp-var-decl");
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
