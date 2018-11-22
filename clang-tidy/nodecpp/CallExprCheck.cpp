//===--- CallExprCheck.cpp - clang-tidy------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CallExprCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void CallExprCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(callExpr().bind("call"), this);
}

void CallExprCheck::check(const MatchFinder::MatchResult &Result) {

  auto expr = Result.Nodes.getNodeAs<CallExpr>("call");

  if(isa<CXXMemberCallExpr>(expr)) {
    //don't check here
    return;
  }

  auto decl = expr->getDirectCallee();
  SourceManager* manager = Result.SourceManager;
  auto eLoc = manager->getExpansionLoc(decl->getLocStart());

  if(!eLoc.isInvalid()) {
    if(!manager->isInSystemHeader(eLoc)) {
      // this is in safe code, then is ok
      return;
    }
  }

  std::string name = decl->getQualifiedNameAsString();
  if(name.substr(0, 9) == "nodecpp::")
    return;

  diag(expr->getExprLoc(), "(S8) unsafe function call is prohibited");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
