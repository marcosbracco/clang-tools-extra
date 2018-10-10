//===--- NakedPtrFromMethodCheck.cpp - clang-tidy--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFromMethodCheck.h"
#include "NakedPtrHelper.h"
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

void NakedPtrFromMethodCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *m = Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");

  //  diag(m->getLocStart(), "call found here!", DiagnosticIDs::Note);

  if (isParentVarDeclOrCompStmtOrReturn(Result.Context, m)) {
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
          if (!isa<DeclRefExpr>(base)) {
            diag(base->getExprLoc(), "Couln't verify base");
            return;
          }

		  const auto *decl = m->getMethodDecl();
          if (!decl) {
            diag(m->getExprLoc(), "callee declaration not available");
            return;
          }

          if (!declRefCheck(Result.Context, lhs, dyn_cast<DeclRefExpr>(base))) {
            diag(base->getExprLoc(),
                 "naked pointer not allowed to extend the context of 'this'");
            return;
          }
          // then check arguments

          auto params = decl->parameters();
          auto ret = decl->getReturnType().getCanonicalType();
          auto args = m->arguments();

          auto it = args.begin();
          auto jt = params.begin();
          while (it != args.end() && jt != params.end()) {
            auto arg = (*jt)->getType().getCanonicalType();
            if (canArgumentGenerateOutput(ret, arg)) {
              if (!checkArgument(Result.Context, lhs, *it)) {
                diag((*it)->getExprLoc(),
                     "couldn't verify naked pointer safety of call argument");
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
    }
  }
  diag(m->getLocStart(), "Couln't verify safety of call");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
