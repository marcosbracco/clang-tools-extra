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

  const auto OwnerType =
      hasType(cxxRecordDecl(hasName("::nodecpp::unique_ptr")));

  const auto OwnCtor = cxxConstructExpr(OwnerType);
  const auto OwnReset = cxxMemberCallExpr(has(
                     memberExpr(allOf(
						 member(hasName("reset")), has(declRefExpr(OwnerType))))));
  
  const auto ptrType =
      hasType(pointerType(pointee(pointerType())));

  //new array is handled separatelly
  const auto badNew = cxxNewExpr(
	  unless(anyOf(isArray(), ptrType, hasParent(OwnCtor),
		  hasParent(OwnReset)))).bind("ownE");

  const auto arrNew =
      cxxNewExpr(isArray()).bind("arrE");

  const auto ptrNew = cxxNewExpr(ptrType).bind("ptrE");

  Finder->addMatcher(badNew, this);
  Finder->addMatcher(arrNew, this);
  Finder->addMatcher(ptrNew, this);
}

void NewExprCheck::check(const MatchFinder::MatchResult &Result) {
 
  if (const auto *m = Result.Nodes.getNodeAs<CXXNewExpr>("ownE"))
	  diag(m->getLocStart(), "new expression must be owned by nodecpp::unique_ptr");
  else if (const auto *m = Result.Nodes.getNodeAs<CXXNewExpr>("arrE"))
    diag(m->getLocStart(),
         "do not use array new expression");
  else if (const auto *m = Result.Nodes.getNodeAs<CXXNewExpr>("ptrE"))
    diag(m->getLocStart(), "do not use new expression of pointers");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
