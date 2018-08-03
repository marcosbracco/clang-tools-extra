//===--- NewNakedPtrCheck.cpp - clang-tidy---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NewNakedPtrCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NewNakedPtrCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(cxxNewExpr().bind("expr"), this);
}

void NewNakedPtrCheck::check(const MatchFinder::MatchResult &Result) {
 
  const auto *MatchedDecl = Result.Nodes.getNodeAs<CXXNewExpr>("expr");
//  const auto *Parent = MatchedDecl->getParent();
//  if (MatchedDecl->getName().startswith("awesome_"))
//    return;
  diag(MatchedDecl->getLocStart(), "new expr found");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
