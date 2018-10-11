//===--- NakedPtrHelper.h - clang-tidy------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRHELPER_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRHELPER_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace nodecpp {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/nodecpp-naked-ptr-from-function.html

bool checkTypeAsSafe(ClangTidyCheck *Check, QualType Qt, SourceLocation Sl,
                     unsigned NakedPtrLevel);
bool checkRecordAsSafe(ClangTidyCheck *Check, const CXXRecordDecl *Decl,
                       unsigned NakedPtrLevel);

inline bool checkRecordAsHeapSafe(ClangTidyCheck *Check,
                                  const CXXRecordDecl *Decl) {
  return checkRecordAsSafe(Check, Decl, 0);
}

inline bool checkTypeAsStackSafe(ClangTidyCheck *Check, QualType Qt,
                                 SourceLocation Sl) {
  return checkTypeAsSafe(Check, Qt, Sl, 1);
}

const BinaryOperator *getParentBinOp(ASTContext *context, const Expr *expr);
const Expr *getParentExpr(ASTContext *context, const Expr *expr);
bool isParentVarDeclOrCompStmtOrReturn(ASTContext *context, const Expr *expr);

const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt);
bool checkArgument(ASTContext *context, const DeclRefExpr *lhs,
                   const Expr *arg);

bool declRefCheck(ASTContext *context, const DeclRefExpr *lhs,
                  const DeclRefExpr *rhs);

bool canArgumentGenerateOutput(QualType out, QualType arg);
} // namespace nodecpp
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRHELPER_H
