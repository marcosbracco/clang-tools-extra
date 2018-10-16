//===--- NakedPtrFieldCheck.cpp - clang-tidy-------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrFieldCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedPtrFieldCheck::registerMatchers(MatchFinder *Finder) {

  // const auto NakedPtr = hasType(hasUnqualifiedDesugaredType(
  //    anyOf(pointerType(), referenceType())));

  // Finder->addMatcher(
  //    declaratorDecl(
  //        hasType(recordDecl(hasDescendant(fieldDecl(NakedPtr).bind("field")),
  //                           unless(hasName("::nodecpp::unique_ptr")),
  //                           unless(hasName("::nodecpp::soft_ptr")))))
  //        .bind("decl"),
  //    this);

  Finder->addMatcher(varDecl(unless(isExpansionInSystemHeader()),
                                 unless(isImplicit()),
                             unless(hasParent(cxxConstructorDecl())))
                         .bind("var"),
                     this);
  //Finder->addMatcher(fieldDecl(hasType(
  //                                 cxxRecordDecl(
  //                                 hasName ("nodecpp::unique_ptr"))))
  //        .bind("fld"),
  //    this);
}

void NakedPtrFieldCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *M = Result.Nodes.getNodeAs<VarDecl>("var");


  if (M) {
    // TODO improve
    if (!M->getLocation().isValid() && !M->getTypeSpecStartLoc().isValid())
      return;

	M->dumpColor();
    auto loc = M->getTypeSpecStartLoc().isValid() ? M->getTypeSpecStartLoc()
                                                  : M->getLocation();
    if (!checkTypeAsStackSafe(this, M->getType(), loc)) {
		//Diag is already printed
//      M->dumpColor();
//      diag(M->getLocation(), "declaration with naked pointer");
      return;
    }
  } else if (auto F = Result.Nodes.getNodeAs<FieldDecl>("fld")) {
	//TODO check that Template argument of unique_ptr is a heap safe
  }
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
