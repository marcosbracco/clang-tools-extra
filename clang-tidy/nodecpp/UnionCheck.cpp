//===--- UnionCheck.cpp - clang-tidy---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UnionCheck.h"
#include "NakedPtrhelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void UnionCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(cxxRecordDecl(isUnion()).bind("rD"), this);
}

void UnionCheck::check(const MatchFinder::MatchResult &Result) {

  auto rD = Result.Nodes.getNodeAs<CXXRecordDecl>("rD");
  // if(rD && rD->hasDefinition() && rD->isUnion()) {
  //   checkUnion(rD, this);
  // }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
