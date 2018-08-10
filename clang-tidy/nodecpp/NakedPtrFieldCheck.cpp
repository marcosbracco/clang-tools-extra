//===--- NakedPtrFieldCheck.cpp - clang-tidy-------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFieldCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrFieldCheck::registerMatchers(MatchFinder *Finder) {

  const auto NakedPtr = hasType(hasUnqualifiedDesugaredType(
      anyOf(pointerType(), referenceType(pointee(pointerType())))));

  Finder->addMatcher(
      declaratorDecl(
          hasType(recordDecl(hasDescendant(fieldDecl(NakedPtr).bind("field")),
                             unless(hasName("::nodecpp::unique_ptr")),
                             unless(hasName("::nodecpp::soft_ptr")))))
          .bind("decl"),
      this);
}

void NakedPtrFieldCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *MatchedDecl = Result.Nodes.getNodeAs<Decl>("decl");

  diag(MatchedDecl->getLocation(),
       "do not use types with naked pointer fields");

  const auto *MatchedField = Result.Nodes.getNodeAs<FieldDecl>("field");

  diag(MatchedField->getLocation(), "field declared here", DiagnosticIDs::Note);
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
