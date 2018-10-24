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

      auto ptr = d->getType().getCanonicalType().getTypePtrOrNull();
      if (!ptr)
        break; // some type error

      if (!ptr->isPointerType()) {
        diag(it->getLocation(), "capture type not allowed");
        break;
      }
      auto r = ptr->getPointeeCXXRecordDecl();
      if (!r->hasAttr<NodeCppNoInstanceAttr>()) {
        diag(it->getLocation(), "capture pointer type not allowed");
        break;
      }

      // we are safe
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

void MayExtendLambdaCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *MatchedExpr = Result.Nodes.getNodeAs<CallExpr>("call");
  const auto *MatchedDecl = Result.Nodes.getNodeAs<CXXMethodDecl>("decl");

  for (unsigned i = 0; i != MatchedDecl->param_size(); ++i) {
    auto p = MatchedDecl->getParamDecl(i);
    if (p->hasAttr<NodeCppMayExtendAttr>()) {

      auto e = MatchedExpr->getArg(i);
      e = ignoreTemporaries(e);
      if (auto ref = dyn_cast_or_null<DeclRefExpr>(e)) {
        //diag(e->getExprLoc(), "argument is declRef");
        auto d = ref->getDecl();
        if (d && isa<VarDecl>(d)) {
          auto vd = cast<VarDecl>(d);
          auto e2 = vd->getInit();
          e2 = ignoreTemporaries(e2);

          if (auto lamb = dyn_cast_or_null<LambdaExpr>(e2)) {
            //checkLambda(lamb);
            checkLambda2(lamb);
            continue;
          }
        }
      } else if (auto lamb = dyn_cast_or_null<LambdaExpr>(e)) {
        //diag(e->getExprLoc(), "argument is LambdaExpr");
        //checkLambda(lamb);
        checkLambda2(lamb);
        continue;
      }
	  // e may be null?
    diag(MatchedExpr->getArg(i)->getExprLoc(), "failed to verify argument");
    }
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
