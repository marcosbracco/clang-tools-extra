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

bool isOwnerName(const std::string &Name);
bool isSafeName(const std::string &Name);
bool isNakedStructName(const std::string &Name);
bool isUnsafeName(const std::string &Name);

bool isParamOnlyType(QualType qt);

bool isNakedStructRecord(const CXXRecordDecl *decl);
bool isStackOnlyType(QualType qt);

bool isSafeRecord(const CXXRecordDecl *decl);
bool isSafeType(QualType qt);

const Expr *getParentExpr(ASTContext *context, const Expr *expr);
const Expr *ignoreTemporaries(const Expr *expr);
const LambdaExpr *getLambda(const Expr *expr);

const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt);
bool checkStack2StackAssignment(ASTContext *context, const Stmt* to, const Stmt* from);
const DeclStmt* getParentDeclStmt(ASTContext *context, const Decl* decl);

bool canArgumentGenerateOutput(QualType out, QualType arg);


class NakedPtrScopeChecker {

  enum OutputScope { Unknown, Stack, Param, This, Global };

  ClangTidyCheck *check; // to write diag messages
  ASTContext *context;


  OutputScope outScope;
  const DeclStmt* outScopeStmt; //only when outScope == Stack

  NakedPtrScopeChecker(ClangTidyCheck *check, ASTContext *context, OutputScope outScope, const Decl* outScopeDecl) :
  check(check), context(context), outScope(outScope)  {
    if(outScopeDecl)
      outScopeStmt = getParentDeclStmt(context, outScopeDecl);
  }


  bool checkDeclRefExpr(const DeclRefExpr *declRef);
  bool checkCallExpr(const CallExpr *call);

public:
  bool checkExpr(const Expr *from);
private:
  static
  std::pair<OutputScope, const Decl*> calculateScope(const Expr* expr);

  static
  std::pair<OutputScope, const Decl*> calculateShorterScope(std::pair<OutputScope, const Decl*> l, std::pair<OutputScope, const Decl*> r);

public:
  static
  NakedPtrScopeChecker makeChecker(ClangTidyCheck *check, ASTContext *context, const Expr* toExpr);

  static
  bool hasThisScope(const Expr* expr);

  static
  bool hasParamScope(const Expr* expr);
};


} // namespace nodecpp
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRHELPER_H
