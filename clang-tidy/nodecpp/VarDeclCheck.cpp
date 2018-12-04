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

  // any type will also pop-up at constructor and operator assignment
  // declaration, we let both of them go
  auto p = getParentMethod(Result.Context, var);
  if(p) {
    if(isa<CXXConstructorDecl>(p))
      return;

    if(auto m = dyn_cast<CXXMethodDecl>(p)) {
      if(m->isCopyAssignmentOperator() || m->isMoveAssignmentOperator())
        return;
    }
  }

  bool isParam = isa<ParmVarDecl>(var);
  auto qt = var->getType().getCanonicalType();

  //unwrap const ref
  if(qt->isReferenceType() && qt.isConstQualified())
    qt = qt->getPointeeType().getCanonicalType();
  
  if(auto u = isUnionType(qt)) {
    if(!checkUnion(u, this))
      diag(var->getLocation(), "referenced from here", DiagnosticIDs::Note);

    return;
  }

  if (isSafeType(qt)) {
    return;
  }

  if(qt->isReferenceType()) {
    //non-const reference only from safe types
    QualType inner = qt->getPointeeType().getCanonicalType();
    if(!isSafeType(inner)) {
      diag(var->getLocation(), "(S5.3) non-const reference of unsafe type is prohibited");
    }

    //this is all for non-const references
    return;
  }

  if(isParam && isParamOnlyType(qt))
    return;

  if(isRawPointerType(qt)) {
    if(!checkRawPointerType(qt, this)) {
      diag(var->getLocation(), "Unsafe raw pointer declaration");
      return;
    }

    
    //params don't need initializer
    if(isParam) {
      diag(var->getLocation(), "(S1.3) raw pointer declaration is prohibited");
      return;
    }
    
    auto e = var->getInit();
    if(!e) {
      diag(var->getLocation(), "raw pointer type must have initializer");
      return;
    }

    //this is all for raw pointer
    diag(var->getLocation(), "(S1.3) raw pointer declaration is prohibited");
    return;
  }


  if(isNakedPointerType(qt)) {

    if(!checkNakedPointerType(qt, this)) {
      diag(var->getLocation(), "unsafe type at naked_ptr declaration");
      return;
    }
    
    //this is all for naked_ptr
    return;
  }

  if(isNakedStructType(qt)) {

    //naked struct internal is checked at other place
    if(var->hasAttr<NodeCppMayExtendAttr>()) {
      diag(var->getLocation(), "may_extend not implemented for naked struct variables yet");
    }

    // this is all for naked_struct
    return;
  }

  if(isImplicitNakedStructType(qt)) {

    //naked struct internal is checked at other place
    if(var->hasAttr<NodeCppMayExtendAttr>()) {
      diag(var->getLocation(), "may_extend not implemented for naked struct variables yet");
    }

    // this is all for iplicit naked struct
    return;
  }

  if(isLambdaType(qt)) {

    //naked struct internal is checked at other place
    if(var->hasAttr<NodeCppMayExtendAttr>()) {
      diag(var->getLocation(), "may_extend not implemented for lambda");
    }

    // this is all for iplicit naked struct
    return;
  }

  auto dh = DiagHelper(this);
  dh.diag(var->getLocation(), "unsafe type at variable declaration");
  isSafeType(qt, dh);

  return;
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
