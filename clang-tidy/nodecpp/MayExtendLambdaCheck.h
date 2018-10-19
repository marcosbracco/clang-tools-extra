//===--- MayExtendLambdaCheck.h - clang-tidy---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_MAYEXTENDLAMBDACHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_MAYEXTENDLAMBDACHECK_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace nodecpp {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/nodecpp-may-extend-lambda.html
class MayExtendLambdaCheck : public ClangTidyCheck {
public:
  MayExtendLambdaCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void checkLambda(const LambdaExpr *lamb);
  void checkLambda2(const LambdaExpr *lamb);

  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace nodecpp
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_MAYEXTENDLAMBDACHECK_H
