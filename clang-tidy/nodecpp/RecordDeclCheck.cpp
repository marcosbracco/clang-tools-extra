//===--- RecordDeclCheck.cpp - clang-tidy----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RecordDeclCheck.h"
#include "NakedPtrhelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void RecordDeclCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(cxxRecordDecl().bind("rd"), this);
}

void RecordDeclCheck::check(const MatchFinder::MatchResult &Result) {

  auto rd = Result.Nodes.getNodeAs<CXXRecordDecl>("rd");
  if(rd && rd->hasDefinition()) {
    auto t = rd->getDescribedClassTemplate();
    
    if(t) // this is a template, don't check here
      return;

    auto k = rd->getTemplateSpecializationKind();
    if(k == TSK_ImplicitInstantiation)
      return;

    if(rd->hasAttr<NodeCppNakedStructAttr>()) {
      checkNakedStructRecord(rd, this);
    }
    else {
      if(isSafeRecord(rd))
        return;

      auto dh = DiagHelper(this);
      dh.diag(rd->getLocation(), "unsafe type declaration");
      isSafeRecord(rd, dh);
    }
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
