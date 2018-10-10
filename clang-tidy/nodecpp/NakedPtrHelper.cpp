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
bool checkRecordAsHeapSafe(ClangTidyCheck *Check, const CXXRecordDecl *Decl) {

  if (!Decl) {
    return false;
  }

  auto F = Decl->fields();
  for (auto It = F.begin(); It != F.end(); ++It) {
    auto Field = *It;
		
	auto Ft = Field->getType().getCanonicalType()
                  .getTypePtrOrNull();
    if (!Ft) {
      Check->diag(Field->getTypeSpecStartLoc(),
                  "failed to verify field safety for heap allocation",
                  DiagnosticIDs::Warning);
      return false;
    }
    else if (Ft->isReferenceType()) {
      Check->diag(
          Field->getTypeSpecStartLoc(),
          "type with reference field can not be allocated on the heap",
          DiagnosticIDs::Warning);
      return false;
	  }
      else if (Ft->isPointerType()) {
        Check->diag(
            Field->getTypeSpecStartLoc(),
            "type with naked pointer field can not be allocated on the heap",
            DiagnosticIDs::Warning);
        return false;
      }
      else if (Ft->isBuiltinType()) {
      continue;
    } else if (Ft->isRecordType()) {
      if (!checkRecordAsHeapSafe(Check, Ft->getAsCXXRecordDecl())) {
        Check->diag(
            Field->getTypeSpecStartLoc(),
            "referenced from here",
            DiagnosticIDs::Note);
        return false;
	  }
    } else {
      Check->diag(
          Field->getTypeSpecStartLoc(),
          "failed to verify field safety for heap allocation",
          DiagnosticIDs::Warning);
      return false;
	}
  }

  auto B = Decl->bases();
  for (auto It = B.begin(); It != B.end(); ++It) {
    auto Base = *It;
    auto Bt = Base.getType().getCanonicalType().getTypePtrOrNull();
    if (!Bt) {
      Check->diag(Base.getLocStart(),
                  "failed to verify base safety for heap allocation",
                  DiagnosticIDs::Warning);
      return false;
    } else if (Bt->isRecordType()) {
      if (!checkRecordAsHeapSafe(Check, Bt->getAsCXXRecordDecl())) {
        Check->diag(Base.getLocStart(), "referenced from here",
                    DiagnosticIDs::Note);
        return false;
      }
	
	} else {
      Check->diag(Base.getLocStart(),
                  "failed to verify base safety for heap allocation",
                  DiagnosticIDs::Warning);
          return false;
	}

  }

  // finally we are safe!
  return true;
}


const BinaryOperator *getParentBinOp(ASTContext *context, const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt == sList.end())
    return nullptr;

  if (auto p = sIt->get<ParenExpr>())
    return getParentBinOp(context, p);
  else
    return sIt->get<BinaryOperator>();
}

const Expr *getParentExpr(ASTContext *context, const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt == sList.end())
    return nullptr;

  if (auto p = sIt->get<ParenExpr>())
    return getParentExpr(context, p);
  else if (auto p = sIt->get<ImplicitCastExpr>())
    return getParentExpr(context, p);
  else
    return sIt->get<Expr>();
}

bool isParentVarDeclOrCompStmtOrReturn(ASTContext *context, const Expr *expr)
{
  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt != sList.end()) {
    if (sIt->get<VarDecl>() != nullptr)
      return true;
    else if (sIt->get<CompoundStmt>() != nullptr)
      return true;
    else if (sIt->get<ReturnStmt>() != nullptr)
      return true;
    else if (sIt->get<ImplicitCastExpr>() != nullptr)
      return isParentVarDeclOrCompStmtOrReturn(
          context, sIt->get<ImplicitCastExpr>());
  }

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
  //out.dump();
  //arg.dump();
  assert(out.isCanonical());
  assert(arg.isCanonical());

  const Type *t = out.getTypePtrOrNull();
  if (!t || !t->isPointerType())
    return false;

  auto qt2 = t->getPointeeType();
  const Type *t2 = qt2.getTypePtrOrNull();
  if (!t2)
    return false;

  const Type *targ = arg.getTypePtrOrNull();
  if (!(targ && (targ->isPointerType() || targ->isReferenceType())))
    return false;

  auto qt2arg = targ->getPointeeType();
  const Type *t2arg = qt2arg.getTypePtrOrNull();
  if (!t2arg)
    return false;

  //assume non builtins, can generate any kind of naked pointers
  if (!t2arg->isBuiltinType())
    return true;

  if (t2arg != t2)
    return false;
  else {
    //qt2.dump();
    //qt2arg.dump();
    return qt2.isAtLeastAsQualifiedAs(qt2arg);
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
