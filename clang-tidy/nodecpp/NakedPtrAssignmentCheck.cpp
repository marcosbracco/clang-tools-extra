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

  const auto *expr = Result.Nodes.getNodeAs<BinaryOperator>("expr");
  const auto *lhs = dyn_cast<DeclRefExpr>(expr->getLHS());
  if (lhs) {
    const auto *lhsDecl = lhs->getDecl();
    if (!lhsDecl) { // shouldn't happend here
      diag(lhs->getExprLoc(), "couldn't verify naked pointer safety");
      return;
    } else {
      const auto *rhs = expr->getRHS()->IgnoreParenImpCasts();
      if (isa<DeclRefExpr>(rhs)) {

        if(!declRefCheck(Result.Context, lhs, dyn_cast<DeclRefExpr>(rhs)))
          diag(lhs->getExprLoc(), "couldn't verify naked pointer safety");

        return;

      } else if (isa<UnaryOperator>(rhs)) {
        const auto *rhsOp = dyn_cast<UnaryOperator>(rhs);
        if (rhsOp->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
          const auto *sub = rhsOp->getSubExpr()->IgnoreParenImpCasts();
          if (isa<DeclRefExpr>(sub)) {
            if(!declRefCheck(Result.Context, lhs, dyn_cast<DeclRefExpr>(sub)))
              diag(lhs->getExprLoc(), "couldn't verify naked pointer safety");

            return;
          }
        }
      } else if (isa<CallExpr>(rhs)) {
		//ok, this case is handled by nodecpp-naked-ptr-from-function
        return;
	  }
    }
  }
  diag(expr->getExprLoc(), "couldn't verify naked pointer safety");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
