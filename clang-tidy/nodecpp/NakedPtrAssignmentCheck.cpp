//===--- NakedPtrAssignmentCheck.cpp - clang-tidy--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrAssignmentCheck.h"
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

/* static */
const Stmt *NakedPtrAssignmentCheck::getParentStmt(ASTContext *context,
                                                   const Stmt *stmt) {

  auto sList = context->getParents(*stmt);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<Stmt>();
  else
    return nullptr;
}

void NakedPtrAssignmentCheck::declRefCheck(ASTContext *context,
                                           const DeclRefExpr *lhs,
                                           const DeclRefExpr *rhs) {
  const auto *lhsDecl = lhs->getDecl();
  if (!lhsDecl) { // shouln't happend here
    diag(lhs->getExprLoc(), "declaration not available");
    return;
  }

  const auto *rhsDecl = rhs->getDecl();
  if (!rhsDecl) { // shouln't happend here
    diag(rhs->getExprLoc(), "declaration not available");
    return;
  }

  auto lList = context->getParents(*lhsDecl);
  auto rList = context->getParents(*rhsDecl);

  auto lIt = lList.begin();
  auto rIt = rList.begin();

  if (lIt != lList.end() && rIt != rList.end()) {

    auto lDeclStmt = lIt->get<DeclStmt>();
    auto rDeclStmt = rIt->get<DeclStmt>();

    if (lDeclStmt != nullptr && rDeclStmt != nullptr) {

      auto rCompStmt = getParentStmt(context, rDeclStmt);
      if (rCompStmt) {

        auto lCompStmt = getParentStmt(context, lDeclStmt);

        while (lCompStmt != nullptr) {

          if (lCompStmt == rCompStmt) {
//            diag(lhs->getExprLoc(), "this is Ok");
            return;
          }

          lCompStmt = getParentStmt(context, lCompStmt);
        }
      }
    }
  }

  // we couldn't verify this is ok, assume the worst
  diag(lhs->getExprLoc(), "do not extend naked pointer context");
}

void NakedPtrAssignmentCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *expr = Result.Nodes.getNodeAs<BinaryOperator>("expr");
  const auto *lhs = dyn_cast<DeclRefExpr>(expr->getLHS());
  if (lhs) {
    const auto *lhsDecl = lhs->getDecl();
    if (!lhsDecl) { // sema error?

      return;
    } else {
      const auto *rhs = expr->getRHS()->IgnoreParenImpCasts();
      if (isa<DeclRefExpr>(rhs)) {

        declRefCheck(Result.Context, lhs, dyn_cast<DeclRefExpr>(rhs));
        return;

      } else if (isa<UnaryOperator>(rhs)) {
        const auto *rhsOp = dyn_cast<UnaryOperator>(rhs);
        if (rhsOp->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
          const auto *sub = rhsOp->getSubExpr()->IgnoreParenImpCasts();
          if (isa<DeclRefExpr>(sub)) {
            declRefCheck(Result.Context, lhs, dyn_cast<DeclRefExpr>(sub));

            return;
          }
        }
      }
    }
  }
  diag(expr->getExprLoc(), "naked pointer assignment not allowed");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
