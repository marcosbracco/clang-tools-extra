//===--- StdFunctionCheck.cpp - clang-tidy---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "StdFunctionCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void StdFunctionCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher(
      cxxOperatorCallExpr(hasOverloadedOperatorName("=")
      ).bind("expr"), this);
}

void StdFunctionCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  // auto expr = Result.Nodes.getNodeAs<CXXOperatorCallExpr>("expr");
  
  // diag(expr->getExprLoc(), "found here");

}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
