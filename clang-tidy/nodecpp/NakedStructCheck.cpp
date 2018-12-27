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
  Finder->addMatcher(expr(hasType(pointerType())).bind("expr"),
      this);

  Finder->addMatcher(expr(hasType(referenceType())).bind("expr"),
      this);
}

void NakedStructCheck::check(const MatchFinder::MatchResult &Result) {
  auto expr = Result.Nodes.getNodeAs<Expr>("expr");

  //first ignore implicits and parens

  if(isa<ExprWithCleanups>(expr))
    return;
  if(isa<MaterializeTemporaryExpr>(expr))
    return;
  if(isa<CXXBindTemporaryExpr>(expr))
    return;
  if(isa<ImplicitCastExpr>(expr))
    return;
  if(isa<ParenExpr>(expr))
    return;
  if(isa<CXXDefaultArgExpr>(expr))
    return;

  // now allow some explicits
  if(isa<CXXThisExpr>(expr))
    return;
  if(isa<CallExpr>(expr))
    return;
  if(isa<CXXNullPtrLiteralExpr>(expr))
    return;
  if(isa<CXXDynamicCastExpr>(expr))
    return;

  if(auto unOp = dyn_cast<UnaryOperator>(expr)) {
    if(unOp->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
      diag(expr->getExprLoc(), "(S1) address-of operator found here");
      return;
    }
  }

  if(auto binOp = dyn_cast<BinaryOperator>(expr)) {
    if(binOp->getOpcode() == BinaryOperatorKind::BO_Assign) {
      //this is ok
      return;
    }
  }

  // this is an error,
  // find the best error message
  if(isa<DeclRefExpr>(expr)) {
      // don't report it here
      return;
  }
  if(isa<CXXNewExpr>(expr)) {
      // don't report here
      return;
  }
  if(isa<CXXDeleteExpr>(expr)) {
      // don't report here
      return;
  }

  if(isa<CXXStaticCastExpr>(expr)) {
    diag(expr->getExprLoc(), "(S1.1) static_cast not allowed");
    return;
  }

  if(isa<CXXReinterpretCastExpr>(expr)) {
    diag(expr->getExprLoc(), "(S1.1) reinterpret_cast not allowed");
    return;
  }

  if(isa<CStyleCastExpr>(expr)) {
    diag(expr->getExprLoc(), "(S1.1) C style cast not allowed");
    return;
  }

  diag(expr->getExprLoc(), "(S1) raw pointer expression not allowed");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
