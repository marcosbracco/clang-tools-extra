//===--- NakedStructCheck.cpp - clang-tidy---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedStructCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedStructCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxRecordDecl(hasAttr(clang::attr::NodeCppNakedStruct)).bind("nake"), this);
}

void NakedStructCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("nake");

  if(decl && decl->hasDefinition()) {
    checkNakedStructRecord(decl, this);    
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
