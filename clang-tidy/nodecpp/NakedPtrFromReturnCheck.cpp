//===--- NakedPtrFromReturnCheck.cpp - clang-tidy--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFromReturnCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrFromReturnCheck::registerMatchers(MatchFinder *Finder) {
  


  Finder->addMatcher(callExpr(hasType(pointerType())).bind("call"), this);
}

/* static */
const BinaryOperator *
NakedPtrFromReturnCheck::getParentBinOp(ASTContext *context,
                                                   const Expr *expr)
{

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<BinaryOperator>();
  else
    return nullptr;
}

/* static */
bool NakedPtrFromReturnCheck::isParentCompStmt(ASTContext *context,
                                                   const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<CompoundStmt>() != nullptr;
  else
    return false;
}

/* static */
bool NakedPtrFromReturnCheck::isParentVarDecl(ASTContext *context,
                                           const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<VarDecl>() != nullptr;
  else
    return false;
}
/* static */
const Stmt *NakedPtrFromReturnCheck::getParentStmt(ASTContext *context,
                                                   const Stmt *stmt) {

  auto sList = context->getParents(*stmt);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<Stmt>();
  else
    return nullptr;
}

bool NakedPtrFromReturnCheck::declRefCheck(ASTContext *context,
                                           const DeclRefExpr *lhs,
                                           const DeclRefExpr *rhs) {
  const auto *lhsDecl = lhs->getDecl();
  if (!lhsDecl) { // shouln't happend here
    diag(lhs->getExprLoc(), "declaration not available");
    return false;
  }

  const auto *rhsDecl = rhs->getDecl();
  if (!rhsDecl) { // shouln't happend here
    diag(rhs->getExprLoc(), "declaration not available");
    return false;
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
            return true;
          }

          lCompStmt = getParentStmt(context, lCompStmt);
        }
      }
    }
  }

  // we couldn't verify this is ok, assume the worst
  diag(rhs->getExprLoc(), "do not extend naked pointer context");
  return false;
}


void NakedPtrFromReturnCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *m = Result.Nodes.getNodeAs<CallExpr>("call");

//  diag(m->getLocStart(), "call found here!", DiagnosticIDs::Note);

  if (isParentCompStmt(Result.Context, m) || isParentVarDecl(Result.Context, m)) {
    // this is ok
    return;

  } else {
    const BinaryOperator *op = getParentBinOp(Result.Context, m);
    if (op && op->getOpcode() == BinaryOperatorKind::BO_Assign) {
      const auto *lhs = dyn_cast<DeclRefExpr>(op->getLHS());
      if (lhs) {
        const auto *lhsDecl = lhs->getDecl();
        if (!lhsDecl) { // sema error?
          diag(lhs->getExprLoc(), "declaration not available");
          return;
        } else {
          bool ok = true;
          auto args = m->arguments();
          for (auto it = args.begin(); it != args.end(); ++it) {
            const auto *rhs = (*it)->IgnoreParenImpCasts();
            if (isa<DeclRefExpr>(rhs)) {

              ok = ok && declRefCheck(Result.Context, lhs,
                                      dyn_cast<DeclRefExpr>(rhs));

            } else if (isa<UnaryOperator>(rhs)) {
              const auto *rhsOp = dyn_cast<UnaryOperator>(rhs);
              if (rhsOp->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
                const auto *sub = rhsOp->getSubExpr()->IgnoreParenImpCasts();
                if (isa<DeclRefExpr>(sub)) {
                  ok = ok && declRefCheck(Result.Context, lhs,
                                          dyn_cast<DeclRefExpr>(sub));
                }
              }
            } else {
              ok = false;
              diag(rhs->getExprLoc(), "Couln't verify argument");
			}
          }
          return;
        }
      }
    }
  }
  diag(m->getLocStart(), "Couln't verify safety of call");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
