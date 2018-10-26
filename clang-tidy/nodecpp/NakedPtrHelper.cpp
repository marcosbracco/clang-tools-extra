//===--- NakedPtrHelper.cpp - clang-tidy------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

bool isOwnerName(const std::string &Name) { return Name == "std::unique_ptr"; }

bool isSafeName(const std::string &Name) {
  return isOwnerName(Name) || Name == "nodecpp::net::Socket" ||
         Name == "nodecpp::net::Server" || Name == "nodecpp::net::Address" ||
         Name == "nodecpp::net::SocketTBase";
}

bool isNakedStructName(const std::string &Name) {
	//nothing here yet
  return false;
}


bool isUnsafeName(const std::string &Name) {
	//nothing here yet
  return isNakedStructName(Name);
}


bool isNakedStructRecord(const CXXRecordDecl *decl) {

  if (!decl) {
    return false;
  }

  // first verify if is a well known class,
  auto name = decl->getQualifiedNameAsString();
  if (isNakedStructName(name))
    return true;


  auto f = decl->fields();
  for (auto it = f.begin(); it != f.end(); ++it) {

    auto qt = (*it)->getType(); 
    if(isSafeType(qt))
      continue;

    if(isStackOnlyType(qt))
      continue;

    return false;
  }

  auto b = decl->bases();
  if(b.begin() != b.end())
    return false;//don't allow any bases yet


  auto m = decl->methods();
  for(auto it = m.begin(); it != m.end(); ++it) {

    auto method = *it;

    if(isa<CXXConstructorDecl>(method) || isa<CXXDestructorDecl>(method))
      continue;

    if(method->isConst())
      continue;

    if(method->isMoveAssignmentOperator() || method->isCopyAssignmentOperator()) {
      if(method->isDeleted())
        continue;
    }
    
    // this is a bad method
    return false;
  }
  
  // finally we are safe!
  return true;

}


bool isStackOnlyType(QualType qt) {

  assert(!isSafeType(qt));

  auto t = qt.getCanonicalType().getTypePtrOrNull();
  if (!t) {
  	// this is a build problem with the type
    // not really our problem yet
    return false; 
  } 
  else if (t->isReferenceType() || t->isPointerType()) {
      return isSafeType(t->getPointeeType());
  } else if (t->isRecordType()) {
    return isNakedStructRecord(t->getAsCXXRecordDecl());
  } else {
    //t->dump();
    return false;
  }
}



bool isSafeRecord(const CXXRecordDecl *decl) {

  if (!decl) {
    return false;
  }

  // first verify if is a well known class,
  auto name = decl->getQualifiedNameAsString();
  if (isSafeName(name))
    return true;
  else if (isUnsafeName(name))
    return false;


  auto F = decl->fields();
  for (auto It = F.begin(); It != F.end(); ++It) {

    if (!isSafeType((*It)->getType()))
      return false;
  }

  auto B = decl->bases();
  for (auto It = B.begin(); It != B.end(); ++It) {

    if (!isSafeType((*It).getType()))
      return false;
  }

  // finally we are safe!
  return true;
}


bool isSafeType(QualType qt) {

  auto t = qt.getCanonicalType().getTypePtrOrNull();
  if (!t) {
  	// this is a build problem with the type
    // not really our problem yet
    return false; 
  } else if (t->isReferenceType() || t->isPointerType()) {
      return false;
  } else if (t->isBuiltinType()) {
    return true;
  } else if (t->isRecordType()) {
    return isSafeRecord(t->getAsCXXRecordDecl());
  } else if (t->isTemplateTypeParmType()) {
    // we will take care at instantiation
    return true;
  } else {
    //t->dump();
    return false;
  }
}



// const BinaryOperator *getParentBinOp(ASTContext *context, const Expr *expr) {

//   auto sList = context->getParents(*expr);

//   auto sIt = sList.begin();

//   if (sIt == sList.end())
//     return nullptr;

//   if (auto p = sIt->get<ParenExpr>())
//     return getParentBinOp(context, p);
//   else
//     return sIt->get<BinaryOperator>();
// }

const Expr *getParentExpr(ASTContext *context, const Expr *expr) {

  auto sList = context->getParents(*expr);

  auto sIt = sList.begin();

  if (sIt == sList.end())
    return nullptr;

  if (auto p = sIt->get<ParenExpr>())
    return getParentExpr(context, p);
  else if (auto p = sIt->get<ImplicitCastExpr>())
    return getParentExpr(context, p);
  else
    return sIt->get<Expr>();
}

const Expr *ignoreTemporaries(const Expr *expr) {
  if (auto b = dyn_cast<CXXBindTemporaryExpr>(expr)) {
    return ignoreTemporaries(b->getSubExpr());
  }else if (auto c = dyn_cast<CXXConstructExpr>(expr)) {
    return ignoreTemporaries(c->getArg(0));
  } else if (auto t = dyn_cast<MaterializeTemporaryExpr>(expr)) {
    return ignoreTemporaries(t->GetTemporaryExpr());
  } else if (auto i = dyn_cast<ImplicitCastExpr>(expr)) {
    return ignoreTemporaries(i->getSubExpr());
  } else if (auto cl = dyn_cast<ExprWithCleanups>(expr)) {
    return ignoreTemporaries(cl->getSubExpr());
  } else {
	  return expr;
  }
} // namespace nodecpp

const LambdaExpr *getLambda(const Expr *expr) {

  if (!expr)
    return nullptr;

  auto e = ignoreTemporaries(expr);

  if (auto lamb = dyn_cast<LambdaExpr>(e)) {
    return lamb;
  } else if (auto ref = dyn_cast<DeclRefExpr>(e)) {
    // diag(e->getExprLoc(), "argument is declRef");
    if (auto d = dyn_cast_or_null<VarDecl>(ref->getDecl())) {
      return getLambda(d->getInit());
    }
  }

  return nullptr;
}

const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt) {

  auto sList = context->getParents(*stmt);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<Stmt>();
  else
    return nullptr;
}

bool checkStack2StackAssignment(ASTContext *context, const Stmt* to, const Stmt* from) {


  if (!to)
    return false;
  
  if(!from)
    return false;

  from = getParentStmt(context, from);
  if (!from)
    return false;


  to = getParentStmt(context, to);
  while (to) {
    if (to == from)
      return true;

    to = getParentStmt(context, to);
  }

  // we couldn't verify this is ok, assume the worst
  return false;
}

const DeclStmt* getParentDeclStmt(ASTContext *context, const Decl* decl) {

  if(!decl)
    return nullptr;

  auto l = context->getParents(*decl);

  if(l.begin() != l.end())
    return l.begin()->get<DeclStmt>();
  else
    return nullptr;
}

bool canArgumentGenerateOutput(QualType out, QualType arg) {
  // out.dump();
  // arg.dump();
  assert(out.isCanonical());
  assert(arg.isCanonical());

  const Type *t = out.getTypePtrOrNull();
  if (!t || !t->isPointerType())
    return false;

  auto qt2 = t->getPointeeType();
  const Type *t2 = qt2.getTypePtrOrNull();
  if (!t2)
    return false;

  const Type *targ = arg.getTypePtrOrNull();
  if (!(targ && (targ->isPointerType() || targ->isReferenceType())))
    return false;

  auto qt2arg = targ->getPointeeType();
  const Type *t2arg = qt2arg.getTypePtrOrNull();
  if (!t2arg)
    return false;

  // assume non builtins, can generate any kind of naked pointers
  if (!t2arg->isBuiltinType())
    return true;

  if (t2arg != t2)
    return false;
  else {
    // qt2.dump();
    // qt2arg.dump();
    return qt2.isAtLeastAsQualifiedAs(qt2arg);
  }
}


bool NakedPtrScopeChecker::checkDeclRefExpr(const DeclRefExpr *declRef) {
  auto fromDecl = declRef->getDecl();
  if (!fromDecl) { // shouln't happend here
    return false;
  }

  if (isa<FieldDecl>(fromDecl)) {
    fromDecl->dumpColor();
    assert(false);
    return false;
  } else if (auto paramVar = dyn_cast<ParmVarDecl>(fromDecl)) {
    switch (outScope) {
    case Stack:
    case Param:
      return true;
    case This:
      return paramVar->hasAttr<NodeCppMayExtendAttr>();
    case Global:
      return false;
    default:
      assert(false);
    }
  } else if (auto var = dyn_cast<VarDecl>(fromDecl)) {
    if (var->hasGlobalStorage())
      return true;
    else {
      if (outScope == Stack) {
        auto stmt = getParentDeclStmt(context, fromDecl);
        return checkStack2StackAssignment(context, outScopeStmt, stmt);
        ;
      }
      return false;
    }
  }

  return false;
}

bool NakedPtrScopeChecker::checkCallExpr(const CallExpr *call) {

  if(!call) { // shouln't happend here
    assert(false);
    return false;
  }
    
  auto callDecl = call->getDirectCallee();
  if (!callDecl) {
    return false;
  }

  if(isa<CXXMemberCallExpr>(call)) {
    auto callee = dyn_cast<MemberExpr>(call->getCallee());

    if(!callee) {
      //can this happend?
      return false;
    }

    if(!checkExpr(callee->getBase())) {
        return false;
    }
  }
  // TODO check if callee is not the first argument

  auto params = callDecl->parameters();
  auto ret = callDecl->getReturnType().getCanonicalType();
  auto args = call->arguments();

  auto it = args.begin();
  auto jt = params.begin();

  if(isa<CXXOperatorCallExpr>(call) && isa<CXXMethodDecl>(callDecl)) {
    // this is an instance method operator call
    // then first argument is actually the instance
    if (it == args.end()) {
      assert(false);
      return false;
    }
    if (!checkExpr(*it)) {
      return false;
    }
    ++it;
  }

  while (it != args.end() && jt != params.end()) {
    auto arg = (*jt)->getType().getCanonicalType();
    if (canArgumentGenerateOutput(ret, arg)) {
      // diag((*it)->getExprLoc(),
      //     "check this argument");

      if (!checkExpr(*it)) {
        return false;
      }
    }
    ++it;
    ++jt;
  }
  if (it != args.end() || jt != params.end()) {
    //TODO diag
    return false;
  }

  // this is ok!
  return true;
}

bool NakedPtrScopeChecker::checkExpr(const Expr *from) {

  if(outScope == Unknown) {
    return false;
  }

  if(!from) { // shouln't happend here
    assert(false);
    return false;
  }

  from = from->IgnoreParenImpCasts();
  if (isa<CXXThisExpr>(from)) {
    switch (outScope) {
    case Stack:
    case Param:
    case This:
      return true;
    case Global:
      return false;
    default:
      assert(false);
      return false;
    }
  } else if (isa<CXXNullPtrLiteralExpr>(from)) {
    return true;
  } else if (auto declRef = dyn_cast<DeclRefExpr>(from)) {
    return checkDeclRefExpr(declRef);
  } else if (auto callExpr = dyn_cast<CallExpr>(from)) {
    // this will check functions, members and both operators
    return checkCallExpr(callExpr);
  } else if (auto member = dyn_cast<MemberExpr>(from)) {
    // TODO verify only members and not methods will get in here
    return checkExpr(member->getBase());
  } else if (auto op = dyn_cast<UnaryOperator>(from)) {
    if (op->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
      return checkExpr(op->getSubExpr());
    }

    from->dumpColor();
    return false;
  } else if (auto defArg = dyn_cast<CXXDefaultArgExpr>(from)) {
    return checkExpr(defArg->getExpr());
  }

  //just in case
  from->dumpColor();
  return false;
}


/* static */
std::pair<NakedPtrScopeChecker::OutputScope, const Decl*>
NakedPtrScopeChecker::calculateScope(const Expr *expr) {

  if(!expr) {
    assert(expr);
    return std::make_pair(Unknown, nullptr);
  }

  expr = expr->IgnoreParenImpCasts();
  if (auto declRef = dyn_cast<DeclRefExpr>(expr)) {
    auto decl = declRef->getDecl();
    if (!decl) { // shouldn't happend here
      return std::make_pair(Unknown, nullptr);
    }

    if (auto parmVar = dyn_cast<ParmVarDecl>(decl)) {

      if (parmVar->hasAttr<NodeCppMayExtendAttr>())
        return std::make_pair(This, nullptr);
      else
        return std::make_pair(Param, nullptr);
    } else if (auto field = dyn_cast<FieldDecl>(decl)) {

      expr->dumpColor();
      assert(false);
      return std::make_pair(Unknown, nullptr);
    } else if (auto var = dyn_cast<VarDecl>(decl)) {
      if (var->hasGlobalStorage())
        return std::make_pair(Global, nullptr);

      return std::make_pair(Stack, decl);
    }
  } else if (auto member = dyn_cast<MemberExpr>(expr)) {
    // TODO verify only members and not methods will get in here
    return calculateScope(member->getBase());
  } else if (isa<CXXThisExpr>(expr)) {
    return std::make_pair(This, nullptr);
  } else if (isa<CXXNullPtrLiteralExpr>(expr)) {
    return std::make_pair(Global, nullptr);
  } else if (auto op = dyn_cast<UnaryOperator>(expr)) {
    if (op->getOpcode() == UnaryOperatorKind::UO_AddrOf) {
      return calculateScope(op->getSubExpr());
    }
  }

  llvm::errs() << "NakedPtrScopeChecker::calculateScope > Unknown\n";
  expr->dumpColor();
  return std::make_pair(Unknown, nullptr);
}

/* static */
NakedPtrScopeChecker NakedPtrScopeChecker::makeChecker(ClangTidyCheck *check,
                                                       ASTContext *context,
                                                       const Expr *toExpr) {

  auto sc = NakedPtrScopeChecker::calculateScope(toExpr);

  return NakedPtrScopeChecker(check, context, sc.first, sc.second);
}

/* static */
bool NakedPtrScopeChecker::hasThisScope(const Expr *expr) {
  auto sc = NakedPtrScopeChecker::calculateScope(expr);

    switch (sc.first) {
    case Unknown:
    case Stack:
    case Param:
      return false;
    case This:
    case Global:
      return true;
    default:
      assert(false);
      return false;
    }

}

} // namespace tidy
} // namespace clang
} // namespace clang
