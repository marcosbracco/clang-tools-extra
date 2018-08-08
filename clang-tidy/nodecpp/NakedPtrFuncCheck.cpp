//===--- NakedPtrFuncCheck.cpp - clang-tidy--------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFuncCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrFuncCheck::registerMatchers(MatchFinder *Finder) {

//  Finder->addMatcher(functionDecl(returns(asString("class Pepe"))).bind("decl"), this);
  //Will match implicit operator new and delete, ignore implicits
  Finder->addMatcher(functionDecl(allOf(unless(isImplicit()), returns(pointerType()))).bind("decl"),
                     this);
  Finder->addMatcher(functionDecl(returns(referenceType(pointee(pointerType())))).bind("decl"), this);

  Finder->addMatcher(
      functionDecl(hasAnyParameter(hasType(referenceType(pointee(pointerType())))))
          .bind("decl"),
      this);
  Finder->addMatcher(functionDecl(hasAnyParameter(hasType(
                                      pointerType(pointee(pointerType())))))
                         .bind("decl"),
                     this);
}

void NakedPtrFuncCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *MatchedDecl = Result.Nodes.getNodeAs<FunctionDecl>("decl");
  //This matchers matches implicit operator new and delete without location, ignore them
  if (MatchedDecl->getLocation().isValid())
    diag(MatchedDecl->getLocation(), "do not extend lifetime of pointers");
  else
    MatchedDecl->dumpColor();
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
