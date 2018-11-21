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


class DiagHelper {
  ClangTidyCheck *check = nullptr;
  DiagnosticIDs::Level level = DiagnosticIDs::Warning;
public:
  DiagHelper() {}
  DiagHelper(ClangTidyCheck *check)
  :check(check) {}

  void diag(SourceLocation loc, StringRef message) {
    if(check) {
      DiagnosticIDs::Level nextLevel = DiagnosticIDs::Note;
      std::swap(level, nextLevel);
      check->diag(loc, message, nextLevel);
    }
  }
};

extern DiagHelper NullDiagHelper;

/// FIXME: Write a short description.
///
bool isOwnerPtrName(const std::string &Name);
bool isSafePtrName(const std::string &Name);

bool isSafeName(const std::string &Name);
bool isNakedStructName(const std::string &Name);
bool isNakedPtrName(const std::string& name);
bool isUnsafeName(const std::string &Name);


bool isParamOnlyType(QualType qt);

bool checkNakedStructRecord(const CXXRecordDecl *decl, ClangTidyCheck *check);
bool isNakedStructType(QualType qt, bool allowImplicit = false);
inline
bool isImplicitNakedStructType(QualType qt) {
  return isNakedStructType(qt, true);
}

QualType getPointeeType(QualType qt);
bool checkNakedPointerType(QualType qt, ClangTidyCheck *check);
bool isNakedPointerType(QualType qt);

bool checkRawPointerType(QualType qt, ClangTidyCheck *check);
bool isRawPointerType(QualType qt);
const ClassTemplateSpecializationDecl* getSafePtrDecl(QualType qt);
bool isSafePtrType(QualType qt);

bool isSafeRecord(const CXXRecordDecl *decl, DiagHelper& dh = NullDiagHelper);
bool isSafeType(QualType qt, DiagHelper& dh = NullDiagHelper);

const CXXRecordDecl* isUnionType(QualType qt);
bool checkUnion(const CXXRecordDecl *decl, ClangTidyCheck *check);



const Expr *getParentExpr(ASTContext *context, const Expr *expr);
const Expr *ignoreTemporaries(const Expr *expr);
const LambdaExpr *getLambda(const Expr *expr);
bool isFunctionPtr(const Expr *expr);

const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt);
const DeclStmt* getParentDeclStmt(ASTContext *context, const Decl* decl);

bool canArgumentGenerateOutput(QualType out, QualType arg);


class NakedPtrScopeChecker {

  enum OutputScope { Unknown, Stack, Param, This };

  ClangTidyCheck *check; // to write diag messages
  ASTContext *context;


  OutputScope outScope;
  const Decl* outScopeDecl; //only when outScope == Stack

  NakedPtrScopeChecker(ClangTidyCheck *check, ASTContext *context, OutputScope outScope, const Decl* outScopeDecl) :
  check(check), context(context), outScope(outScope), outScopeDecl(outScopeDecl)  {}

  bool checkStack2StackAssignment(const Decl* fromDecl);

  bool checkDeclRefExpr(const DeclRefExpr *declRef);
  bool checkCallExpr(const CallExpr *call);
  bool checkCXXConstructExpr(const CXXConstructExpr *construct);

public:
  bool checkExpr(const Expr *from);
private:
  static
  std::pair<OutputScope, const Decl*> calculateScope(const Expr* expr);

  // static
  // std::pair<OutputScope, const Decl*> calculateShorterScope(std::pair<OutputScope, const Decl*> l, std::pair<OutputScope, const Decl*> r);

public:
  static
  NakedPtrScopeChecker makeChecker(ClangTidyCheck *check, ASTContext *context, const Expr* toExpr);
  static
  NakedPtrScopeChecker makeThisScopeChecker(ClangTidyCheck *check);
  static
  NakedPtrScopeChecker makeParamScopeChecker(ClangTidyCheck *check);
};





} // namespace nodecpp
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_NODECPP_NAKEDPTRHELPER_H
