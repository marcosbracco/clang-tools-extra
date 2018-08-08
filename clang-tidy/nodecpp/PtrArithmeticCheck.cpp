//===--- PtrArithmeticCheck.cpp - clang-tidy-------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PtrArithmeticCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void PtrArithmeticCheck::registerMatchers(MatchFinder *Finder) {
	//copied from ProBoundsPointerArithmeticCheck.cpp

  //if (!getLangOpts().CPlusPlus)
  //  return;

  // Flag all operators +, -, +=, -=, ++, -- that result in a pointer
  Finder->addMatcher(
      binaryOperator(
          anyOf(hasOperatorName("+"), hasOperatorName("-"),
                hasOperatorName("+="), hasOperatorName("-=")),
          hasType(pointerType()),
          unless(hasLHS(ignoringImpCasts(declRefExpr(to(isImplicit()))))))
          .bind("expr"),
      this);

  Finder->addMatcher(
      unaryOperator(anyOf(hasOperatorName("++"), hasOperatorName("--")),
                    hasType(pointerType()))
          .bind("expr"),
      this);

  // Array subscript on a pointer (not an array) is also pointer arithmetic
  Finder->addMatcher(
      arraySubscriptExpr(
          hasBase(ignoringImpCasts(
              anyOf(hasType(pointerType()),
                    hasType(decayedType(hasDecayedType(pointerType())))))))
          .bind("expr"),
      this);
}

void PtrArithmeticCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MatchedExpr = Result.Nodes.getNodeAs<Expr>("expr");

  diag(MatchedExpr->getExprLoc(), "do not use pointer arithmetic");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
