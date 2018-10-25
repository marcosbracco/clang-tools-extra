//===--- NakedPtrAssignmentCheck.cpp - clang-tidy--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrAssignmentCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrAssignmentCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      binaryOperator(hasOperatorName("="),
          hasType(pointerType())/*,
          unless(hasLHS(ignoringImpCasts(declRefExpr(to(isImplicit())))))*/)
          .bind("expr"),
      this);
}

void NakedPtrAssignmentCheck::check(const MatchFinder::MatchResult &Result) {

  auto expr = Result.Nodes.getNodeAs<BinaryOperator>("expr");


  auto checker = NakedPtrScopeChecker::makeChecker(this, Result.Context, expr->getLHS());

  if(!checker.checkExpr(expr->getRHS()))
    diag(expr->getExprLoc(), "assignment of naked pointer may extend scope");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
