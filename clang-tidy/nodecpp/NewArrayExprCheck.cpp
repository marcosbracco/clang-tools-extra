//===--- NewArrayExprCheck.cpp - clang-tidy--------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NewArrayExprCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NewArrayExprCheck::registerMatchers(MatchFinder *Finder) {
  if (!getLangOpts().CPlusPlus)
    return;

  const auto arrNew = cxxNewExpr(isArray()).bind("expr");

  Finder->addMatcher(arrNew, this);
}

void NewArrayExprCheck::check(const MatchFinder::MatchResult &Result) {

	const auto *MatchedDecl = Result.Nodes.getNodeAs<CXXNewExpr>("expr");

  diag(MatchedDecl->getLocStart(),
       "do not use new array expression");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
