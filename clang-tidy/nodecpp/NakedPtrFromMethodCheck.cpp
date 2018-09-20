//===--- NakedPtrFromMethodCheck.cpp - clang-tidy--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFromMethodCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrFromMethodCheck::registerMatchers(MatchFinder *Finder) {
  
  if (!getLangOpts().CPlusPlus)
    return;

  Finder->addMatcher(cxxMemberCallExpr(hasType(pointerType())).bind("call"), this);
}

/* static */
const BinaryOperator *
NakedPtrFromMethodCheck::getParentBinOp(ASTContext *context, const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<BinaryOperator>();
  else
    return nullptr;
}

/* static */
bool NakedPtrFromMethodCheck::isParentCompStmt(ASTContext *context,
                                               const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<CompoundStmt>() != nullptr;
  else
    return false;
}

/* static */
bool NakedPtrFromMethodCheck::isParentVarDecl(ASTContext *context,
                                              const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<VarDecl>() != nullptr;
  else
    return false;
}
/* static */
const Stmt *NakedPtrFromMethodCheck::getParentStmt(ASTContext *context,
                                                   const Stmt *stmt) {

  auto sList = context->getParents(*stmt);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<Stmt>();
  else
    return nullptr;
}

bool NakedPtrFromMethodCheck::declRefCheck(ASTContext *context,
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
  diag(rhs->getExprLoc(), "couldn't verify naked pointer safety");
  return false;
}

void NakedPtrFromMethodCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *m = Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");

  //  diag(m->getLocStart(), "call found here!", DiagnosticIDs::Note);

  if (isParentCompStmt(Result.Context, m) ||
      isParentVarDecl(Result.Context, m)) {
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
          // first check instance
          const auto *callee = dyn_cast<MemberExpr>(m->getCallee());
          if (!callee) {
            diag(lhs->getExprLoc(), "member expr not available");
            return;
          }
          const auto *base = callee->getBase()->IgnoreParenImpCasts();
          if (isa<DeclRefExpr>(base)) {
            if (!declRefCheck(Result.Context, lhs, dyn_cast<DeclRefExpr>(base)))
              return;

          } else {
            diag(base->getExprLoc(), "Couln't verify base");
            return;
          }

          // then check arguments

          auto args = m->arguments();
          for (auto it = args.begin(); it != args.end(); ++it) {
            const auto *rhs = (*it)->IgnoreParenImpCasts();
			if (isa <CXXNullPtrLiteralExpr>(rhs)) {
				;//nothing to do
			}
            else if (isa<DeclRefExpr>(rhs)) {
              if (!declRefCheck(Result.Context, lhs,
                                dyn_cast<DeclRefExpr>(rhs)))
                return;

            } else if (isa<UnaryOperator>(rhs)) {
              const auto *rhsOp = dyn_cast<UnaryOperator>(rhs);
              if (rhsOp->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
                const auto *sub = rhsOp->getSubExpr()->IgnoreParenImpCasts();
                if (isa<DeclRefExpr>(sub)) {
                  if (!declRefCheck(Result.Context, lhs,
                                    dyn_cast<DeclRefExpr>(sub)))
                    return;
                }
              }
            } else {
              diag(rhs->getExprLoc(), "Couln't verify argument");
              return;
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
