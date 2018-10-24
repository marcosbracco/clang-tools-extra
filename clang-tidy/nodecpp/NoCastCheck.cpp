//===--- NoCastCheck.cpp - clang-tidy--------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NoCastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NoCastCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(cxxConstCastExpr().bind("cast"), this);
  Finder->addMatcher(cxxStaticCastExpr(unless(hasDestinationType(rValueReferenceType()))).bind("cast"), this);
  Finder->addMatcher(cxxReinterpretCastExpr().bind("cast"), this);

  Finder->addMatcher(
      cStyleCastExpr(
          unless(allOf(hasDestinationType(voidType()),
                       hasSourceExpression(integerLiteral(equals(0))))))
          .bind("cast"),
      this);
}

void NoCastCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *MatchedCast = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  
  diag(MatchedCast->getExprLoc(), "do not use cast");
}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
