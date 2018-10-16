//===--- NewExprCheck.cpp - clang-tidy-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrHelper.h"
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
  /*
  const auto OwnerType =
      hasType(cxxRecordDecl(hasName("::nodecpp::unique_ptr")));

  const auto OwnCtor = cxxConstructExpr(OwnerType);
  const auto OwnReset = cxxMemberCallExpr(has(
                     memberExpr(allOf(
                                                 member(hasName("reset")),
  has(declRefExpr(OwnerType))))));

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
  */

  Finder->addMatcher(
      cxxNewExpr(unless(isExpansionInSystemHeader())).bind("new"), this);
}

void NewExprCheck::check(const MatchFinder::MatchResult &Result) {
  /*
  if (const auto *m = Result.Nodes.getNodeAs<CXXNewExpr>("ownE"))
          diag(m->getLocStart(), "new expression must be owned by
  nodecpp::unique_ptr"); else if (const auto *m =
  Result.Nodes.getNodeAs<CXXNewExpr>("arrE")) diag(m->getLocStart(), "do not use
  array new expression"); else if (const auto *m =
  Result.Nodes.getNodeAs<CXXNewExpr>("ptrE")) diag(m->getLocStart(), "do not use
  new expression of pointers");
        */
  auto m = Result.Nodes.getNodeAs<CXXNewExpr>("new");

  if (m->isArray()) {
    diag(m->getLocStart(), "do not use array new expression");
    return;
  } else if (m->getNumPlacementArgs() != 0) {
    diag(m->getLocStart(), "do not use placement new expression");
    return;
  }

  auto t = m->getAllocatedType().getCanonicalType().getTypePtrOrNull();
  if (!t) {
    diag(m->getLocStart(), "failed to verify type safety for heap allocation");
    return;
  } else if (t->isPointerType()) {
    diag(m->getLocStart(), "do not use new expression of pointer");
    return;
  } else if (t->isBuiltinType()) {
	//this is ok!
 
  } else if (t->isRecordType()) {
    if (!checkRecordAsHeapSafe(this, t->getAsCXXRecordDecl())) {
      diag(m->getAllocatedTypeSourceInfo()->getTypeLoc().getBeginLoc(), "referenced from here",
                  DiagnosticIDs::Note);
      return;
    }
  } else {
    diag(m->getLocStart(), "failed to verify type safety for heap allocation");
    return;
  }

  //type is ok, verify is owned by unique_ptr

  auto Parent = getParentExpr(Result.Context, m);
  if (Parent) {
	  if (auto Constructor = dyn_cast<CXXConstructExpr>(Parent)) {
		auto Decl = Constructor->getConstructor()->getParent();
            auto ClassName = Decl->getQualifiedNameAsString();
                if (isOwnerName(ClassName)) {
			//this is ok!
		  return;
		  }
	  } else if (auto MemberCall = dyn_cast<CXXMemberCallExpr>(Parent)) {

		auto Method = MemberCall->getMethodDecl();
		auto Decl = MemberCall->getMethodDecl()->getParent();
                auto MethodName = Method->getNameAsString();
                auto ClassName = Decl->getQualifiedNameAsString();
                if (MethodName == "reset" && isOwnerName(ClassName)) {
		  // this is ok!
		  return;
		}
          }
  }

  diag(m->getLocStart(), "new expresion must be owned by a unique_ptr");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
