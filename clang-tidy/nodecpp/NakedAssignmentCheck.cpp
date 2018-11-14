//===--- NakedAssignmentCheck.cpp - clang-tidy-----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedAssignmentCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedAssignmentCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxOperatorCallExpr(hasOverloadedOperatorName("=")
      ).bind("expr"), this);
}

void NakedAssignmentCheck::check(const MatchFinder::MatchResult &Result) {

  auto expr = Result.Nodes.getNodeAs<CXXOperatorCallExpr>("expr");

  if(expr->getNumArgs() == 2) {
    auto left = expr->getArg(0);
    if(isNakedPointerType(left->getType())) {
        auto checker = NakedPtrScopeChecker::makeChecker(this, Result.Context, left);

        if(!checker.checkExpr(expr->getArg(1)))
            diag(expr->getExprLoc(), "assignment of naked_ptr may extend scope");
    }
    else if(isNakedStructType(left->getType())) {
        auto checker = NakedPtrScopeChecker::makeChecker(this, Result.Context, left);

        if(!checker.checkExpr(expr->getArg(1)))
            diag(expr->getExprLoc(), "assignment of naked_struct may extend scope");
    }
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
