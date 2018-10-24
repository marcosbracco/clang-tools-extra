//===--- VarDeclCheck.cpp - clang-tidy-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "VarDeclCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void VarDeclCheck::registerMatchers(MatchFinder *Finder) {

  // Finder->addMatcher(
  //     varDecl(unless(anyOf(
  //       hasParent(cxxConstructorDecl()),
  //                          hasParent(cxxMethodDecl(isCopyAssignmentOperator())),

  //                          hasParent(cxxMethodDecl(isMoveAssignmentOperator()))
  //                          )))
  //         .bind("var"),
  //     this);

  Finder->addMatcher(varDecl().bind("var"), this);
}

void VarDeclCheck::check(const MatchFinder::MatchResult &Result) {

  auto var = Result.Nodes.getNodeAs<VarDecl>("var");
  assert(var);

  auto p = getParentDecl(Result.Context, var);
  if(p) {
    if(isa<CXXConstructorDecl>(p))
      return;

    if(auto m = dyn_cast<CXXMethodDecl>(p)) {
      if(m->isCopyAssignmentOperator() || m->isMoveAssignmentOperator())
        return;
    }
  }

  if (isSafeType(var->getType()))
    return;
  
  if(isStackOnlyType(var->getType()))
    return;

  diag(var->getTypeSpecStartLoc(), "unsafe type at variable declaration");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
