//===--- StaticStorageCheck.cpp - clang-tidy-------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "StaticStorageCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void StaticStorageCheck::registerMatchers(MatchFinder *Finder) {

//  if (!getLangOpts().CPlusPlus)
//    return;
 
  Finder->addMatcher(varDecl(hasStaticStorageDuration(), unless(isConstexpr())).bind("decl"), this);
  Finder->addMatcher(varDecl(hasThreadStorageDuration()).bind("decl"), this);
}

void StaticStorageCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *MatchedDecl = Result.Nodes.getNodeAs<VarDecl>("decl");

  diag(MatchedDecl->getLocation(),
       "do not use global, static or thread_local variables");
      //<< MatchedDecl
      //<< FixItHint::CreateInsertion(MatchedDecl->getLocation(), "awesome_");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
