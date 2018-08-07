//===--- NewExprCheck.cpp - clang-tidy-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

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

  const auto OwnerDecl = cxxRecordDecl(hasName("::std::unique_ptr"));
//    const auto FakeDecl = cxxRecordDecl(hasName("fake"));
  const auto OwnerType = hasType(OwnerDecl);
//  const auto IsNakedPtrType = hasType(pointerType());

  const auto OwnCtor = cxxConstructExpr(OwnerType);
  const auto OwnReset = cxxMemberCallExpr(has(
                     memberExpr(allOf(
						 member(hasName("reset")), has(declRefExpr(OwnerType))))));
  
  //new array is handled separatelly
  const auto badNew = cxxNewExpr(
	  unless(anyOf(isArray(), hasParent(OwnCtor),
		  hasParent(OwnReset)))).bind("expr");

  Finder->addMatcher(badNew, this);
}

void NewExprCheck::check(const MatchFinder::MatchResult &Result) {
 
  const auto *MatchedDecl = Result.Nodes.getNodeAs<CXXNewExpr>("expr");

  diag(MatchedDecl->getLocStart(), "new expression must be owned by unique_ptr");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
