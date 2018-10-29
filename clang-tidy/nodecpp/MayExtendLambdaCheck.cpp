//===--- MayExtendLambdaCheck.cpp - clang-tidy-----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MayExtendLambdaCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void MayExtendLambdaCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMemberCallExpr(callee(cxxMethodDecl(hasAnyParameter(hasAttr(
                                           clang::attr::NodeCppMayExtend))).bind("decl")))
                         .bind("call"),
                     this);
}

void MayExtendLambdaCheck::checkLambda(const LambdaExpr *lamb) {
  
  auto caps = lamb->captures();
  for (auto it = caps.begin(); it != caps.end(); ++it) {
    switch (it->getCaptureKind()) {
    case LCK_This:
    case LCK_StarThis:
      // this is ok
      break;
    case LCK_ByCopy: {
      auto d = it->getCapturedVar();
      if (isSafeType(d->getType()))
        break;

      if (d->hasGlobalStorage())
        break;

      if(d->hasAttr<NodeCppMayExtendAttr>())
        break;

      diag(it->getLocation(), "unsafe capture to extend scope");
    } break;
    case LCK_ByRef:
      diag(it->getLocation(), "capture by reference not allowed");
      break;
    case LCK_VLAType:
      diag(it->getLocation(), "capture by array not allowed");
      break;
    default:
      diag(it->getLocation(), "unknow capture kind not allowed");
    }
  }
}

void MayExtendLambdaCheck::checkLambda2(const LambdaExpr *lamb) {

  auto fields = lamb->getLambdaClass()->fields();
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    if (isSafeType(it->getType()))
      continue;

    auto ptr = it->getType().getCanonicalType().getTypePtrOrNull();
    if (!ptr)
      continue; // some type error, not our problem

    if (!ptr->isPointerType()) {
      diag(it->getLocation(), "capture type not allowed");
      continue;
    }
    auto r = ptr->getPointeeCXXRecordDecl();
    if (!r->hasAttr<NodeCppNoInstanceAttr>()) {
      diag(it->getLocation(), "capture pointer type not allowed");
      continue;
    }

    // we are safe continue with next
  }
}

/* static */
const LambdaExpr *MayExtendLambdaCheck::getLambda(const Expr *expr) {

  if (!expr)
    return nullptr;

  auto e = ignoreTemporaries(expr);

  if (auto lamb = dyn_cast<LambdaExpr>(e)) {
    return lamb;
  } else if (auto ref = dyn_cast<DeclRefExpr>(e)) {
    // diag(e->getExprLoc(), "argument is declRef");
    if (auto d = dyn_cast_or_null<VarDecl>(ref->getDecl())) {
      return getLambda(d->getInit());
    }
  }

  return nullptr;
}

/* static */
bool MayExtendLambdaCheck::hasRealLifeAsThis(const Expr* expr) {

  if(!expr) {
    assert(false);
    return false;
  }

  expr = expr->IgnoreParenImpCasts();

  if (isa<CXXThisExpr>(expr)) {
    return true;
  }
  else if (auto member = dyn_cast<MemberExpr>(expr)) {
    //this is allways 'arrow', so check first
    if(isa<CXXThisExpr>(member->getBase()))
      return true;

    if(member->isArrow())
      return false;

    return hasRealLifeAsThis(member->getBase());
  }

  return false;
}

void MayExtendLambdaCheck::check(const MatchFinder::MatchResult &Result) {

  auto call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");
  auto decl = Result.Nodes.getNodeAs<CXXMethodDecl>("decl");

  auto callee = call->getCallee();
  if(!hasRealLifeAsThis(callee)) {
    diag(callee->getExprLoc(), "methods with [[may_extend_to_this]] attribute can be called only on members that share the lifetime of this");
    return;
  }

  for (unsigned i = 0; i != decl->param_size(); ++i) {
    auto p = decl->getParamDecl(i);
    if (p->hasAttr<NodeCppMayExtendAttr>()) {

      auto e = call->getArg(i);
      if(isFunctionPtr(e)) {
        continue;
      } else if(auto lamb = getLambda(e)) {
        checkLambda(lamb);
        continue;
      } else {
        auto ch = NakedPtrScopeChecker::makeThisScopeChecker(this);
        if(ch.checkExpr(e))
          continue;
      }
	    // e may be null?
      diag(e->getExprLoc(), "is not safe to extend argument scope to 'this'");
    }
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
