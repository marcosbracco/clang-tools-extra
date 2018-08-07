//===--- NakedPtrCheck.cpp - clang-tidy------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      fieldDecl(hasType(pointerType())).bind("decl"),
      this);
}

void NakedPtrCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *MatchedDecl = Result.Nodes.getNodeAs<FieldDecl>("decl");

  diag(MatchedDecl->getLocation(),
       "do not use naked pointer fields");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
