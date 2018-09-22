//===--- NakedPtrFromFunctionCheck.cpp - clang-tidy------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFromFunctionCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrFromFunctionCheck::registerMatchers(MatchFinder *Finder) {
  


  Finder->addMatcher(callExpr(hasType(pointerType())).bind("call"), this);
}

void NakedPtrFromFunctionCheck::check(const MatchFinder::MatchResult &Result) {
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
          auto args = m->arguments();
          for (auto it = args.begin(); it != args.end(); ++it) {

			if (!checkArgument(Result.Context, lhs, *it)) {
              diag((*it)->getExprLoc(), "couldn't verify naked pointer safety of call argument");
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
