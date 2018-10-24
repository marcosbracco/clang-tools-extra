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

bool isOwnerName(const std::string &Name);
bool isSafeName(const std::string &Name);
bool isNakedStructName(const std::string &Name);
bool isUnsafeName(const std::string &Name);

bool checkTypeAsSafe(ClangTidyCheck *Check, QualType Qt, SourceLocation Sl,
                     unsigned NakedPtrLevel);
bool checkRecordAsSafe(ClangTidyCheck *Check, const CXXRecordDecl *Decl,
                       unsigned NakedPtrLevel);


inline bool checkTypeAsStackSafe(ClangTidyCheck *Check, QualType Qt,
                                 SourceLocation Sl) {
  return checkTypeAsSafe(Check, Qt, Sl, 1);
}


bool isNakedStructRecord(const CXXRecordDecl *decl);
bool isStackOnlyType(QualType qt);

bool isSafeRecord(const CXXRecordDecl *decl);
bool isSafeType(QualType qt);

const BinaryOperator *getParentBinOp(ASTContext *context, const Expr *expr);
const Expr *getParentExpr(ASTContext *context, const Expr *expr);
const Expr *ignoreTemporaries(const Expr *expr);
bool isParentVarDeclOrCompStmtOrReturn(ASTContext *context, const Expr *expr);

const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt);
bool checkStack2StackAssignment(ASTContext *context, const Stmt* to, const Stmt* from);
const DeclStmt* getParentDeclStmt(ASTContext *context, const Decl* decl);



bool checkArgument(ASTContext *context, const DeclRefExpr *lhs,
                   const Expr *arg);

bool declRefCheck(ASTContext *context, const DeclRefExpr *lhs,
                  const DeclRefExpr *rhs);

bool canArgumentGenerateOutput(QualType out, QualType arg);


class NakedPtrScopeChecker {
  ClangTidyCheck *check; // to write diag messages
  ASTContext *context;

  enum OutputScope { Stack, Param, This, Global };

  OutputScope outScope;
  const DeclStmt* outScopeStmt; //only when outScope == Stack

  NakedPtrScopeChecker(ClangTidyCheck *check, ASTContext *context, OutputScope outScope, const DeclStmt* outScopeStmt) :
  check(check), context(context), outScope(outScope), outScopeStmt(outScopeStmt) {}


  bool checkInputExpr(const Expr *from);


};

/*
class matcher_HeapSafeTypeMatcher
    : public ::clang::ast_matchers::internal::MatcherInterface<Type> {
public:
  explicit matcher_HeapSafeTypeMatcher() = default;
  bool matches(const Type &Node,
               ::clang::ast_matchers::internal::ASTMatchFinder *Finder,
               ::clang::ast_matchers::internal::BoundNodesTreeBuilder *Builder)
      const override;
};
inline ::clang::ast_matchers::internal::Matcher<Type> heapSafeType() {
  return ::clang::ast_matchers::internal::makeMatcher(
      new matcher_HeapSafeTypeMatcher());
}
inline bool matcher_HeapSafeTypeMatcher::matches(
    const Type &Node, ::clang::ast_matchers::internal::ASTMatchFinder *Finder,
    ::clang::ast_matchers::internal::BoundNodesTreeBuilder *Builder) const {
  return false;
}
*/


} // namespace nodecpp
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRHELPER_H
