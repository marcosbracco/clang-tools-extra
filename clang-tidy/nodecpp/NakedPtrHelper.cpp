//===--- NakedPtrHelper.cpp - clang-tidy------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

const BinaryOperator *
getParentBinOp(ASTContext *context,
                                                   const Expr *expr)
{

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<BinaryOperator>();
  else
    return nullptr;
}

bool isParentCompStmt(ASTContext *context,
                                                   const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<CompoundStmt>() != nullptr;
  else
    return false;
}

bool isParentVarDecl(ASTContext *context,
                                           const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<VarDecl>() != nullptr;
  else
    return false;
}
const Stmt *getParentStmt(ASTContext *context,
                                                   const Stmt *stmt) {

  auto sList = context->getParents(*stmt);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<Stmt>();
  else
    return nullptr;
}


bool declRefCheck(ASTContext *context,
                                           const DeclRefExpr *lhs,
                                           const DeclRefExpr *rhs) {
  const auto *lhsDecl = lhs->getDecl();
  if (!lhsDecl) { // shouln't happend here
    return false;
  }

  const auto *rhsDecl = rhs->getDecl();
  if (!rhsDecl) { // shouln't happend here
    return false;
  }

  if (isa<ParmVarDecl>(rhsDecl)) {
	// if rhs is a parameter, is always ok
    return true;
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
  return false;
}

bool checkArgument(ASTContext *context, const DeclRefExpr *lhs,
                   const Expr *arg) {

  arg = arg->IgnoreParenImpCasts();
  if (isa<CXXNullPtrLiteralExpr>(arg) || isa<CXXDefaultArgExpr>(arg)) {
    return true; // nothing to do
  } else if (isa<DeclRefExpr>(arg)) {
    return declRefCheck(context, lhs, dyn_cast<DeclRefExpr>(arg));

  } else if (isa<UnaryOperator>(arg)) {
    const auto *rhsOp = dyn_cast<UnaryOperator>(arg);
    if (rhsOp->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
      const auto *sub = rhsOp->getSubExpr()->IgnoreParenImpCasts();
      if (isa<DeclRefExpr>(sub)) {
        return declRefCheck(context, lhs, dyn_cast<DeclRefExpr>(sub));
      }
    }
  }

  return false;
}

bool canArgumentGenerateOutput(QualType out, QualType arg) {
  out.dump();
  arg.dump();

  bool outIsBuiltIn = false;
  const Type *t = out.getTypePtrOrNull();
  if (t && t->isPointerType()) {
    const Type *t2 = t->getPointeeType().getTypePtrOrNull();
    if (t2 && t2->isBuiltinType()) {
      outIsBuiltIn = true;
    }
  } else {
    return false;
  }

  const Type *targ = arg.getTypePtrOrNull();
  if (targ && (targ->isPointerType() || targ->isReferenceType())) {
    const Type *t2arg = targ->getPointeeType().getTypePtrOrNull();
    if (t2arg && t2arg->isBuiltinType()) {
      if (!outIsBuiltIn) {
        return false;
      } else {
        return true;
      }
    } else {
      return true;
    }
  } else {
    return false;
  }
}


/*
bool NakedPtrFromFunctionCheck::check(const MatchFinder::MatchResult &Result) {
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
            if (isa<CXXNullPtrLiteralExpr>(rhs) || isa<CXXDefaultArgExpr>(rhs)) {
              ; // nothing to do
            }
            else if (isa<DeclRefExpr>(rhs)) {

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
*/

} // namespace nodecpp
} // namespace tidy
} // namespace clang
