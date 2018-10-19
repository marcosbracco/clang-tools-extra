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

bool isOwnerName(const std::string &Name) { return Name == "std::unique_ptr"; }

bool isSafeName(const std::string &Name) {
  return isOwnerName(Name) || Name == "nodecpp::net::Socket" ||
         Name == "nodecpp::net::Server" || Name == "nodecpp::net::Address" ||
         Name == "nodecpp::net::SocketTBase";
}

bool isUnsafeName(const std::string &Name) {
	//nothing here yet
  return false;
}


bool checkTypeAsSafe(ClangTidyCheck *Check, QualType Qt, SourceLocation Sl,
                     unsigned NakedPtrLevel) {

  auto Ft = Qt.getCanonicalType().getTypePtrOrNull();
  if (!Ft) {
    Check->diag(Sl, "failed to verify type safety", DiagnosticIDs::Warning);
    return false;
  } else if (Ft->isReferenceType()) {
    if (NakedPtrLevel == 0) {
      Check->diag(Sl, "reference type not allowed", DiagnosticIDs::Warning);
      return false;
    } else {
      return checkTypeAsSafe(Check, Ft->getPointeeType(), Sl, --NakedPtrLevel);
    }
  } else if (Ft->isPointerType()) {
    if (NakedPtrLevel == 0) {
      Check->diag(Sl, "naked pointer type not allowed", DiagnosticIDs::Warning);
      return false;
    } else {
      return checkTypeAsSafe(Check, Ft->getPointeeType(), Sl, --NakedPtrLevel);
    }
  } else if (Ft->isBuiltinType()) {
    return true;
  } else if (Ft->isRecordType()) {
    if (checkRecordAsSafe(Check, Ft->getAsCXXRecordDecl(), NakedPtrLevel)) {
      return true;
    } else {
      Check->diag(Sl, "referenced from here", DiagnosticIDs::Note);
      return false;
    }
  } else if (Ft->isTemplateTypeParmType()) {
    // we will take care at instantiation
    return true;
  } else {
    //Ft->dump();
    Check->diag(Sl, "failed to verify type safety", DiagnosticIDs::Warning);
    return false;
  }
}

bool checkRecordAsSafe(ClangTidyCheck *Check, const CXXRecordDecl *Decl,
                       unsigned NakedPtrLevel) {

  if (!Decl) {
    return false;
  }

  // first verify if is a well known class,
  auto name = Decl->getQualifiedNameAsString();
  if (isSafeName(name))
    return true;
  else if (isUnsafeName(name))
    return false;

  auto F = Decl->fields();
  for (auto It = F.begin(); It != F.end(); ++It) {

    if (!checkTypeAsSafe(Check, (*It)->getType(), (*It)->getTypeSpecStartLoc(),
                         NakedPtrLevel)) {
      Check->diag((*It)->getTypeSpecStartLoc(), "failed to verify field",
                  DiagnosticIDs::Note);
      return false;
    }
  }

  auto B = Decl->bases();
  for (auto It = B.begin(); It != B.end(); ++It) {
    auto Base = *It;

    if (!checkTypeAsSafe(Check, (*It).getType(), (*It).getLocStart(),
                         NakedPtrLevel)) {
      Check->diag(Base.getLocStart(), "failed to verify base",
                  DiagnosticIDs::Note);
      return false;
    }
  }

  // finally we are safe!
  return true;
}


bool isSafeRecord(const CXXRecordDecl *decl) {

  if (!decl) {
    return false;
  }

  // first verify if is a well known class,
  auto name = decl->getQualifiedNameAsString();
  if (isSafeName(name))
    return true;
  else if (isUnsafeName(name))
    return false;


  auto F = decl->fields();
  for (auto It = F.begin(); It != F.end(); ++It) {

    if (!isSafeType((*It)->getType()))
      return false;
  }

  auto B = decl->bases();
  for (auto It = B.begin(); It != B.end(); ++It) {

    if (!isSafeType((*It).getType()))
      return false;
  }

  // finally we are safe!
  return true;
}


bool isSafeType(QualType qt) {

  auto t = qt.getCanonicalType().getTypePtrOrNull();
  if (!t) {
	// this is a problem with the type, not our problem yet
	  // came back again when code builds without errors
    return true; 
  } else if (t->isReferenceType() || t->isPointerType()) {
      return false;
  } else if (t->isBuiltinType()) {
    return true;
  } else if (t->isRecordType()) {
    return isSafeRecord(t->getAsCXXRecordDecl());
  } else if (t->isTemplateTypeParmType()) {
    // we will take care at instantiation
    return true;
  } else {
    //t->dump();
    return false;
  }
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

const Expr *ignoreTemporaries(const Expr *expr) {
  if (auto b = dyn_cast<CXXBindTemporaryExpr>(expr)) {
    return ignoreTemporaries(b->getSubExpr());
  }else if (auto c = dyn_cast<CXXConstructExpr>(expr)) {
    return ignoreTemporaries(c->getArg(0));
  } else if (auto t = dyn_cast<MaterializeTemporaryExpr>(expr)) {
    return ignoreTemporaries(t->GetTemporaryExpr());
  } else if (auto i = dyn_cast<ImplicitCastExpr>(expr)) {
    return ignoreTemporaries(i->getSubExpr());
  } else if (auto cl = dyn_cast<ExprWithCleanups>(expr)) {
    return ignoreTemporaries(cl->getSubExpr());
  } else {
	  return expr;
  }
} // namespace nodecpp

bool isParentVarDeclOrCompStmtOrReturn(ASTContext *context, const Expr *expr) {
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
      return isParentVarDeclOrCompStmtOrReturn(context,
                                               sIt->get<ImplicitCastExpr>());
  }

  return false;
}

const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt) {

  auto sList = context->getParents(*stmt);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<Stmt>();
  else
    return nullptr;
}

bool declRefCheck(ASTContext *context, const DeclRefExpr *lhs,
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
  // out.dump();
  // arg.dump();
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

  // assume non builtins, can generate any kind of naked pointers
  if (!t2arg->isBuiltinType())
    return true;

  if (t2arg != t2)
    return false;
  else {
    // qt2.dump();
    // qt2arg.dump();
    return qt2.isAtLeastAsQualifiedAs(qt2arg);
  }
}

/*
bool NakedPtrFromFunctionCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *m = Result.Nodes.getNodeAs<CallExpr>("call");

//  diag(m->getLocStart(), "call found here!", DiagnosticIDs::Note);

  if (isParentCompStmt(Result.Context, m) || isParentVarDecl(Result.Context, m))
{
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
            if (isa<CXXNullPtrLiteralExpr>(rhs) || isa<CXXDefaultArgExpr>(rhs))
{ ; // nothing to do
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

} // namespace tidy
} // namespace clang
} // namespace clang
