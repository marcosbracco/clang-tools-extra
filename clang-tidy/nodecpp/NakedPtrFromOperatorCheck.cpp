//===--- NakedPtrFromOperatorCheck.cpp - clang-tidy------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFromOperatorCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrFromOperatorCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxOperatorCallExpr(
          anyOf(hasType(pointerType()), hasType(autoType(hasDeducedType(pointerType())))))
          .bind("call"),
      this);
}

void NakedPtrFromOperatorCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *m = Result.Nodes.getNodeAs<CXXOperatorCallExpr>("call");

//  m->dumpColor();

  if (isParentCompStmt(Result.Context, m) ||
      isParentVarDecl(Result.Context, m)) {
    // this is always ok
    return;
  }

  const BinaryOperator *op = getParentBinOp(Result.Context, m);
  if (op && op->getOpcode() == BinaryOperatorKind::BO_Assign) {
    const auto *lhs = dyn_cast<DeclRefExpr>(op->getLHS());
    if (lhs) {
      const auto *lhsDecl = lhs->getDecl();
      if (!lhsDecl) { // sema error?
        diag(lhs->getExprLoc(), "lhs declaration not available");
        return;
      }

	  const auto *decl = m->getDirectCallee();
      if (!decl) {
        diag(m->getExprLoc(), "callee declaration not available");
        return;
      }

      // then check arguments
      if (m->getNumArgs() < 2) {
        //  // something is very wrong
        diag(m->getExprLoc(), "bad argument count");
        return;
      }

      auto params = decl->parameters();
      auto ret = decl->getReturnType().getCanonicalType();
      auto args = m->arguments();

      auto it = args.begin();
      auto jt = params.begin();

      if (isa<CXXMethodDecl>(decl)) {
        // then first args is actually the base instance
        if (it == args.end()) {
          diag(m->getExprLoc(), "something is very wrong");
          return;
        }

        auto base = (*it)->IgnoreParenImpCasts();
        if (!isa<DeclRefExpr>(base)) {
          diag(base->getExprLoc(), "Couln't verify base");
          return;
        }

		auto rDecl = cast<CXXMethodDecl>(decl)->getParent();
        if (!rDecl->isLambda()) {
			if (!declRefCheck(Result.Context, lhs, dyn_cast<DeclRefExpr>(base))) {
			  diag(base->getExprLoc(), "naked pointer not allowed to "
									   "extend the context of 'this'");
			  return;
			}
        }

        ++it;
      } 


      while (it != args.end() && jt != params.end()) {
        auto arg = (*jt)->getType().getCanonicalType();
        if (canArgumentGenerateOutput(ret, arg)) {

          if (!checkArgument(Result.Context, lhs, *it)) {
            diag((*it)->getExprLoc(),
                 "couldn't verify naked pointer safety of argument");
            return;
          }
		}
        ++it;
        ++jt;
      }
      if (it == args.end() && jt == params.end()) {
        // this is ok!
        return;
      }
    }
  }

  diag(m->getLocStart(), "Couln't verify safety of call");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
