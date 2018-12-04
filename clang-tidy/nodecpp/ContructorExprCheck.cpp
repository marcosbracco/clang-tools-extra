//===--- ContructorExprCheck.cpp - clang-tidy------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ContructorExprCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void ContructorExprCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxConstructExpr().bind("ctor"), this);
}

void ContructorExprCheck::check(const MatchFinder::MatchResult &Result) {
 
  if(auto ctor = Result.Nodes.getNodeAs<CXXConstructExpr>("ctor")) {

    QualType qt = ctor->getType().getCanonicalType();
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

    ctor->dump();
    auto dh = DiagHelper(this);
    dh.diag(ctor->getExprLoc(), "unsafe type contructor");
    isSafeType(qt, dh);
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
