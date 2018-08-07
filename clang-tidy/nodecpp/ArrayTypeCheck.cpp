//===--- ArrayTypeCheck.cpp - clang-tidy-----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ArrayTypeCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void ArrayTypeCheck::registerMatchers(MatchFinder *Finder) {


  Finder->addMatcher(typeLoc(loc(arrayType())).bind("type"), this);
}

void ArrayTypeCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *MatchedType = Result.Nodes.getNodeAs<TypeLoc>("type");

  diag(MatchedType->getLocStart(), "do not use arrays");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
