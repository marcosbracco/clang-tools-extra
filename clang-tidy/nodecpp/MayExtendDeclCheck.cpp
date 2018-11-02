//===--- MayExtendDeclCheck.cpp - clang-tidy-------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MayExtendDeclCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTLambda.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void MayExtendDeclCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(functionDecl(hasAnyParameter(hasAttr(
                                           attr::NodeCppMayExtend))).bind("decl"),
                     this);

  Finder->addMatcher(lambdaExpr().bind("lamb"), this);
}

void MayExtendDeclCheck::check(const MatchFinder::MatchResult &Result) {
  
  auto l = Result.Nodes.getNodeAs<LambdaExpr>("lamb");

  //TODO create a real matcher
  if(l) {
    auto decl = l->getCallOperator();
    for (unsigned i = 0; i != decl->param_size(); ++i) {
      auto p = decl->getParamDecl(i);
      if (p->hasAttr<NodeCppMayExtendAttr>()) {
        diag(p->getLocation(), "attribute [may_extend_to_this] can't be used on lambdas");
        return;
      }
    }
    //this lambda is ok
    return;
  }

  auto d = Result.Nodes.getNodeAs<FunctionDecl>("decl");


  auto m = dyn_cast<CXXMethodDecl>(d);
  if(!m) {
    diag(d->getLocation(), "attribute [may_extend_to_this] can only be used on non-static member methods");
    return;
  }

  if(m->isStatic()) {
    diag(d->getLocation(), "attribute [may_extend_to_this] can't be used on static member methods");
    return;
  } else if(isLambdaCallOperator(m)) {
    //this is not matching anything
    diag(d->getLocation(), "attribute [may_extend_to_this] can't be used on lambdas");
    return;
  }

  //this is ok
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
