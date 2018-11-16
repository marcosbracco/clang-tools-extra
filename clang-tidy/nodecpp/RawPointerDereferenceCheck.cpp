//===--- RawPointerDereferenceCheck.cpp - clang-tidy-----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RawPointerDereferenceCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void RawPointerDereferenceCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(memberExpr(isArrow()).bind("arr"), this);

  Finder->addMatcher(unaryOperator(hasOperatorName("*"),
                                   hasUnaryOperand(hasType(pointerType())))
                         .bind("op"),
                     this);
}

void RawPointerDereferenceCheck::check(const MatchFinder::MatchResult &Result) {

  auto expr = Result.Nodes.getNodeAs<MemberExpr>("arr");
  if(expr) {
    auto base = expr->getBase()->IgnoreParenImpCasts();
    if(isa<CXXThisExpr>(base))
      return;
    else if(isa<CallExpr>(base))
      return;

    diag(expr->getExprLoc(), "do not dereference raw pointer");
    return;
  }

  auto op = Result.Nodes.getNodeAs<UnaryOperator>("op");

  diag(op->getExprLoc(), "do not dereference raw pointer");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
