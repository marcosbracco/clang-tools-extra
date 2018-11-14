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

const CXXMethodDecl *VarDeclCheck::getParentMethod(ASTContext *context, const VarDecl *var) {

  auto sList = context->getParents(*var);

  for(auto sIt = sList.begin(); sIt != sList.end(); ++sIt) {
    if(auto t = sIt->get<TypeLoc>()) {
      auto sList2 = context->getParents(*t);
      for(auto sIt2 = sList2.begin(); sIt2 != sList2.end(); ++sIt2) {
        if (auto d = sIt2->get<CXXMethodDecl>())
          return d;
      }
    }
    else if (auto d = sIt->get<CXXMethodDecl>())
      return d;
  }

  return nullptr;
}


void VarDeclCheck::check(const MatchFinder::MatchResult &Result) {

  auto var = Result.Nodes.getNodeAs<VarDecl>("var");
  assert(var);

  auto p = getParentMethod(Result.Context, var);
  if(p) {
    if(isa<CXXConstructorDecl>(p))
      return;

    if(auto m = dyn_cast<CXXMethodDecl>(p)) {
      if(m->isCopyAssignmentOperator() || m->isMoveAssignmentOperator())
        return;
    }
  }

  auto qt = var->getType();
  if (isSafeType(qt))
    return;

  if(isa<ParmVarDecl>(var) && isParamOnlyType(qt))
    return;

  if(isRawPointerType(qt)) {
    if(checkRawPointerType(qt, this)) {
      //params don't need initializer
      if(isa<ParmVarDecl>(var))
        return;
      
      auto e = var->getInit();
      if(!e) {
        diag(var->getLocation(), "raw pointer type must have initializer");
        return;
      }

      if(var->hasAttr<NodeCppMayExtendAttr>()) {
        //then we must check scope
        auto sc = NakedPtrScopeChecker::makeThisScopeChecker(this);
        if(!sc.checkExpr(e)) {
          diag(var->getLocation(), "initializer not allowed to may_extend declaration");
          return;
        }
      }
    }

    //this is all for raw pointer
    return;
  }

  //a variable not a parameter
  if(isNakedPointerType(qt)) {

    if(checkNakedPointerType(qt, this)) {
      if(var->hasAttr<NodeCppMayExtendAttr>()) {
        auto e = var->getInit();
        //then we must check scope
        auto sc = NakedPtrScopeChecker::makeThisScopeChecker(this);
        if(!sc.checkExpr(e)) {
          diag(var->getLocation(), "initializer not allowed to may_extend declaration");
          return;
        }
      }
    }

    //this is all for naked_ptr
    return;
  }

  if(isNakedStructType(qt)) {
    // check constructor
    if(var->hasAttr<NodeCppMayExtendAttr>()) {
      diag(var->getLocation(), "may_extend not implemented for naked struct variables yet");
    }

    // this is all for naked_struct
    return;
  }

  diag(var->getLocation(), "unsafe type at variable declaration");
  return;
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
