//===--- NewExprCheck.cpp - clang-tidy-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrHelper.h"
#include "NewExprCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NewExprCheck::registerMatchers(MatchFinder *Finder) {

  if (!getLangOpts().CPlusPlus)
    return;

  Finder->addMatcher(
      cxxNewExpr().bind("new"), this);
}

void NewExprCheck::check(const MatchFinder::MatchResult &Result) {

  auto m = Result.Nodes.getNodeAs<CXXNewExpr>("new");

  if (m->isArray()) {
    diag(m->getLocStart(), "do not use array new expression");
    return;
  } else if (m->getNumPlacementArgs() != 0) {
    diag(m->getLocStart(), "do not use placement new expression");
    return;
  }

  if (!isSafeType(m->getAllocatedType())) {
    diag(m->getLocStart(), "type is not safe for heap allocation");
    return;
  }

  if (isNoInstanceType(m->getAllocatedType())) {
    diag(m->getLocStart(),
         "type with attribute can be instantiated from safe code");
    return;
  }

  // type is ok, verify is owned by unique_ptr

  auto parent = getParentExpr(Result.Context, m);
  if (parent) {
//    parent->dumpColor();
    if (auto constructor = dyn_cast<CXXConstructExpr>(parent)) {
      auto decl = constructor->getConstructor()->getParent();
      auto name = decl->getQualifiedNameAsString();
      if (isOwnerName(name)) {
        // this is ok!
        return;
      }
    } else if (auto member = dyn_cast<CXXMemberCallExpr>(parent)) {

      auto method = member->getMethodDecl();
      auto decl = member->getMethodDecl()->getParent();
      auto methodName = method->getNameAsString();
      auto className = decl->getQualifiedNameAsString();
      if (methodName == "reset" && isOwnerName(className)) {
        // this is ok!
        return;
      }
    }
  }

  diag(m->getLocStart(), "new expresion must be owned by a unique_ptr");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
