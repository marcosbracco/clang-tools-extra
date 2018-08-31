//===--- NakedPtrAssignmentCheck.cpp - clang-tidy--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrAssignmentCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrAssignmentCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      binaryOperator(hasOperatorName("="),
          hasType(pointerType())/*,
          unless(hasLHS(ignoringImpCasts(declRefExpr(to(isImplicit())))))*/)
          .bind("expr"),
      this);
}

void NakedPtrAssignmentCheck::declRefCheck(const DeclRefExpr *lhs,
                                           const DeclRefExpr *rhs) {

    const auto *lhsDecl = lhs->getDecl();
	if (!lhsDecl) { // sema error?
      diag(lhs->getExprLoc(), "declaration not available",
           DiagnosticIDs::Remark);
      return;
	} 

	const auto *rhsDecl = rhs->getDecl();
    if (!rhsDecl) { // sema error?
        diag(rhs->getExprLoc(), "declaration not available", DiagnosticIDs::Remark);
        return;
    }

	const auto *declCtx = lhsDecl->getDeclContext();
    if (declCtx != rhsDecl->getDeclContext()) {
          diag(rhs->getExprLoc(), "do not extend naked pointer context (different DeclContext)",
               DiagnosticIDs::Warning);
      return;
	}

	auto its = declCtx->decls();
        for (auto it = its.begin(); it != its.end(); ++it) {
          if (*it == rhsDecl) {
			// rhs found first, this is ok
            return;
          } else if (*it == lhsDecl) {
			  //lhs found first, this is bad
            diag(lhs->getExprLoc(),
                 "do not extend naked pointer context",
                 DiagnosticIDs::Warning);
                          return;          
		  }
		}
        diag(lhs->getExprLoc(), "do not extend naked pointer context (shouldn't reach here)",
             DiagnosticIDs::Warning);
}

void NakedPtrAssignmentCheck::check(const MatchFinder::MatchResult &Result) {

	const auto *expr = Result.Nodes.getNodeAs<BinaryOperator>("expr");
    const auto* lhs = dyn_cast<DeclRefExpr>(expr->getLHS());
    if (lhs) {
      const auto *lhsDecl = lhs->getDecl();
          if (!lhsDecl) {// sema error?
			  
           return;
          } 
          else {
            const auto *rhs = expr->getRHS()->IgnoreParenImpCasts();
			if (isa<DeclRefExpr>(rhs)) {

			  declRefCheck(lhs, dyn_cast<DeclRefExpr>(rhs));
              return;

			}
			else if (isa<UnaryOperator>(rhs)) {
				const auto* rhsOp = dyn_cast<UnaryOperator>(rhs);
				if (rhsOp->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
                                  const auto *sub = rhsOp->getSubExpr()
                                                        ->IgnoreParenImpCasts();
                                  if (isa<DeclRefExpr>(sub)) {
                                    declRefCheck(lhs,
                                                 dyn_cast<DeclRefExpr>(sub));
                                  
									  return;
								  }
						  
				}

			}
		  }
	}
    diag(expr->getExprLoc(),
             "naked pointer assignment not allowed");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
