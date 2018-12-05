//===--- TemporaryExprCheck.cpp - clang-tidy-------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TemporaryExprCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void TemporaryExprCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(cxxTemporaryObjectExpr().bind("tmp"), this);
}

void TemporaryExprCheck::check(const MatchFinder::MatchResult &Result) {
 
  if(auto tmp = Result.Nodes.getNodeAs<CXXTemporaryObjectExpr>("tmp")) {

    QualType qt = tmp->getType().getCanonicalType();
    if (isSafeType(qt))
      return;
    if(isParamOnlyType(qt))
      return;

    if(isRawPointerType(qt))
      return;
    if(isNakedPointerType(qt))
      return;
    
    if(isNakedStructType(qt))
      return;

    if(isImplicitNakedStructType(qt))
      return;

    if(isLambdaType(qt))
      return;

//    tmp->dump();
    auto dh = DiagHelper(this);
    dh.diag(tmp->getExprLoc(), "unsafe type at temporary expression");
    isSafeType(qt, dh);
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
