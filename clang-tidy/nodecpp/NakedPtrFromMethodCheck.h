//===--- NakedPtrFromMethodCheck.h - clang-tidy------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRFROMMETHODCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRFROMMETHODCHECK_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace nodecpp {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/nodecpp-naked-ptr-from-method.html
class NakedPtrFromMethodCheck : public ClangTidyCheck {
public:
  NakedPtrFromMethodCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;

  static const BinaryOperator *getParentBinOp(ASTContext *context,
                                              const Expr *expr);
  static bool isParentCompStmt(ASTContext *context, const Expr *expr);
  static bool isParentVarDecl(ASTContext *context, const Expr *expr);

  static const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt);
  bool declRefCheck(ASTContext *context, const DeclRefExpr *lhs,
                    const DeclRefExpr *rhs);
 
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace nodecpp
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRFROMMETHODCHECK_H
