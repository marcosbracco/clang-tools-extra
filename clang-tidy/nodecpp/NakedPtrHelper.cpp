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

bool isOwnerName(const std::string &Name) {
  return Name == "std::unique_ptr" || Name == "nodecpp::unique_ptr"; }

bool isSafeName(const std::string &Name) {
  return isOwnerName(Name) || Name == "nodecpp::safe_ptr" ||
  Name == "nodecpp::net::Socket" ||
         Name == "nodecpp::net::Server" || Name == "nodecpp::net::Address" ||
         Name == "nodecpp::net::SocketTBase";
}

bool isNakedStructName(const std::string &Name) {
	//nothing here yet
  return false;
}

bool isNakedPtrName(const std::string& name) {
  return name == "nodecpp::naked_ptr";
}

bool isUnsafeName(const std::string &Name) {
	//nothing here yet
  return isNakedStructName(Name) || isNakedPtrName(Name);
}

bool isParamOnlyType(QualType qt) {

  assert(!isSafeType(qt));

  auto t = qt.getCanonicalType().getTypePtrOrNull();
  if (!t) {
  	// this is a build problem with the type
    // not really our problem yet
    return false; 
  } 
  else if (t->isReferenceType() || t->isPointerType()) {
      return false;
  } else if (t->isRecordType()) {
    auto decl = t->getAsCXXRecordDecl();
    if (!decl || !decl->hasDefinition())
      return false;

    auto name = decl->getQualifiedNameAsString();
    if (name == "std::function")
      return true;
  }
  
  return false;
}

bool checkNakedStructRecord(const CXXRecordDecl *decl, ClangTidyCheck *check) {

  //on debug break here
  assert(decl);
  assert(decl->hasDefinition());

  if (!decl || !decl->hasDefinition()) {
    return false;
  }

  //we check explicit and implicit here
  bool hasAttr = decl->hasAttr<NodeCppNakedStructAttr>();

  bool checkInits = false;
  std::list<const FieldDecl*> missingInitializers;
  auto f = decl->fields();
  for (auto it = f.begin(); it != f.end(); ++it) {

    auto qt = (*it)->getType(); 
    if(isSafeType(qt))
      continue;

    if(isNakedPointerType(qt)) {
      if(checkInits && !(*it)->hasInClassInitializer())
        missingInitializers.push_back(*it);
  
      continue;
    }

    if(isNakedStructType(qt))
      continue;

    if(check)
      check->diag((*it)->getLocation(), "member not allowed at naked struct");
      
    return false;
  }

  auto b = decl->bases();
  if(b.begin() != b.end()) {
    if(check)
      check->diag(b.begin()->getLocStart(), "inheritance not allowed at naked struct");
    return false;//don't allow any bases yet
  }

  auto m = decl->methods();
  for(auto it = m.begin(); it != m.end(); ++it) {

    auto method = *it;

    if(isa<CXXDestructorDecl>(method))
      continue;

    if(auto ctr = dyn_cast<CXXConstructorDecl>(method)) {
      if(!missingInitializers.empty()) {
        auto inits = ctr->inits();
        std::list<const FieldDecl*> decls(missingInitializers);
        for(auto jt = inits.begin();jt != inits.end(); ++jt) {
          if((*jt)->isMemberInitializer()) {
              decls.remove((*jt)->getMember());
          }
        }

        if(!decls.empty()) {
          if(check)
            check->diag(method->getLocation(), "constructor of naked struct has uninitialized raw pointers");
          return false;
        }
      }
      continue;
    }

    if(method->isConst())
      continue;

    if(method->isMoveAssignmentOperator() || method->isCopyAssignmentOperator()) {
      if(method->isDeleted())
        continue;

      if(hasAttr && method->isDefaulted())
        continue;
    }
    
    if(check)
      check->diag(method->getLocation(), "method not allowed at naked struct");
    return false;
  }
  
  // finally we are safe!
  return true;
}


bool isNakedStructType(QualType qt) {

  assert(!isSafeType(qt));

  qt = qt.getCanonicalType();
 
  if (qt->isRecordType()) {
    auto decl = qt->getAsCXXRecordDecl();

    if (!decl || !decl->hasDefinition())
      return false;

    // first verify if is a well known class,
    auto name = decl->getQualifiedNameAsString();
    if (isNakedStructName(name))
      return true;
  
    //if it has attribute, the some other rule
    // must have verified it
    if(decl->hasAttr<NodeCppNakedStructAttr>())
      return true;
  }
  
    //t->dump();
  return false;
}

bool checkRawPointerType(QualType qt, ClangTidyCheck *check) {
  return true;
}

bool isRawPointerType(QualType qt) {

  assert(!isSafeType(qt));

  return qt.getCanonicalType()->isPointerType();
}


bool checkNakedPointerType(QualType qt, ClangTidyCheck *check) {
  return true;
 
  // assert(isNakedPointerType(qt));
  
  // qt = qt.getCanonicalType();

  // auto decl = qt->getAsCXXRecordDecl();

  // if (!decl || !decl->hasDefinition())
  //   return false;

  // // first verify if is a well known class,
  // auto name = decl->getQualifiedNameAsString();
  // if (!isNakedPtrName(name))
  //   return false;

  // if (auto t = qt->getAs<TemplateSpecializationType>()) {
    
  //   if(t->getNumArgs() >= 1) {

  //     auto argt = t->getArg(0).getAsType();
  //     if(isSafeType(argt)) {
  //       return true;
  //     }
  //   }
  // }
  //   t->getAsCXXRecordDecl();
  //   t->get

  //     template
  //     //TODO verify template type, it must be safe      
  //     return true;
  //   }
  // }
  
  //   //t->dump();
  // return false;
}



bool isNakedPointerType(QualType qt) {
  
  assert(!isSafeType(qt));

  if (qt->isRecordType()) {
    auto decl = qt->getAsCXXRecordDecl();

    if (!decl || !decl->hasDefinition())
      return false;

    // first verify if is a well known class,
    auto name = decl->getQualifiedNameAsString();
    if (isNakedPtrName(name))
      return true;
  }
  
    //t->dump();
  return false;
}



bool isSafeRecord(const CXXRecordDecl *decl) {

  if (!decl || !decl->hasDefinition()) {
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

  qt = qt.getCanonicalType();
  if (qt->isReferenceType() || qt->isPointerType()) {
      return false;
  } else if (qt->isBuiltinType()) {
    return true;
  } else if (qt->isRecordType()) {
    return isSafeRecord(qt->getAsCXXRecordDecl());
  } else if (qt->isTemplateTypeParmType()) {
    // we will take care at instantiation
    return true;
  } else {
    //t->dump();
    return false;
  }
}

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


bool isFunctionPtr(const Expr *expr) {

  if (!expr)
    return false;

  auto e = ignoreTemporaries(expr);

  if (auto op = dyn_cast<UnaryOperator>(e)) {
    if (op->getOpcode() != UnaryOperatorKind::UO_AddrOf)
      return false;

    e = op->getSubExpr();
  }

  if (auto ref = dyn_cast<DeclRefExpr>(e)) {
    return isa<FunctionDecl>(ref->getDecl());
  }

  return false;
}



const Stmt *getParentStmt(ASTContext *context, const Stmt *stmt) {

  auto sList = context->getParents(*stmt);

  auto sIt = sList.begin();

  if (sIt != sList.end())
    return sIt->get<Stmt>();
  else
    return nullptr;
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

  //until properly updated for naked_ptr template
  return true;

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

bool NakedPtrScopeChecker::checkStack2StackAssignment(const Decl *fromDecl) {

  if(!context || !outScopeDecl) {
    check->diag(fromDecl->getLocStart(), "Internal checker error, please report", DiagnosticIDs::Error);
    return false;
  }
  
  auto fromStmt = getParentDeclStmt(context, fromDecl);
  if (!fromStmt)
    return false;

  auto toStmt = getParentDeclStmt(context, outScopeDecl);
  if (!toStmt)
    return false;

  auto from = getParentStmt(context, fromStmt);
  if (!from)
    return false;

  auto to = getParentStmt(context, toStmt);
  while (to) {
    if (to == from)
      return true;

    to = getParentStmt(context, to);
  }

  // we couldn't verify this is ok, assume the worst
  return false;
}

bool NakedPtrScopeChecker::checkDeclRefExpr(const DeclRefExpr *declRef) {
  auto fromDecl = declRef->getDecl();
  if (!fromDecl) { // shouln't happend here
    return false;
  }

  if (isa<FieldDecl>(fromDecl)) {
    fromDecl->dumpColor();
    assert(false);
  } else if (auto paramVar = dyn_cast<ParmVarDecl>(fromDecl)) {
    switch (outScope) {
    case Stack:
    case Param:
      return true;
    case This:
      return paramVar->hasAttr<NodeCppMayExtendAttr>();
    default:
      assert(false);
    }
  } else if (auto var = dyn_cast<VarDecl>(fromDecl)) {
    if (var->hasGlobalStorage())
      return true;
    else if(var->hasAttr<NodeCppMayExtendAttr>()) {
      return true;
    }
    else {
      if (outScope == Stack) {
        return checkStack2StackAssignment(fromDecl);
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

bool NakedPtrScopeChecker::checkCXXConstructExpr(const CXXConstructExpr *construct) {

  auto args = construct->arguments();

  for(auto it = args.begin(); it != args.end(); ++it) {

    if (!checkExpr(*it)) {
      return false;
    }
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
  } else if (auto op = dyn_cast<ConditionalOperator>(from)) {
    //check both branches
    if(checkExpr(op->getTrueExpr()))
      return checkExpr(op->getFalseExpr());
    else    
      return false;
  } else if(auto cast = dyn_cast<CastExpr>(from)) {
    return checkExpr(cast->getSubExpr());
  } else if(auto tmp = dyn_cast<MaterializeTemporaryExpr>(from)) {
    return checkExpr(tmp->GetTemporaryExpr());
  } else if(auto construct = dyn_cast<CXXConstructExpr>(from)) {
    return checkCXXConstructExpr(construct);
  }


  //just in case
  from->dumpColor();
  return false;
}


/* static */
std::pair<NakedPtrScopeChecker::OutputScope, const Decl*>
NakedPtrScopeChecker::calculateScope(const Expr *expr) {

  // Scope is only calculated for the expression acting as lhs
  // all other expressions are checked againt them
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
      if (var->hasGlobalStorage()) //globals can't be changed
        return std::make_pair(Unknown, nullptr);
      else if(var->hasAttr<NodeCppMayExtendAttr>())
        return std::make_pair(This, nullptr);
      else
        return std::make_pair(Stack, decl);
    }
  } else if (auto member = dyn_cast<MemberExpr>(expr)) {
    // TODO verify only members and not methods will get in here
    // TODO also verify is a hard member and not a pointer / unique_ptr
    return calculateScope(member->getBase());
  } else if (isa<CXXThisExpr>(expr)) {
    return std::make_pair(This, nullptr);
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
NakedPtrScopeChecker NakedPtrScopeChecker::makeThisScopeChecker(ClangTidyCheck *check) {

  return NakedPtrScopeChecker(check, nullptr, This, nullptr);
}
/* static */
NakedPtrScopeChecker NakedPtrScopeChecker::makeParamScopeChecker(ClangTidyCheck *check) {

  return NakedPtrScopeChecker(check, nullptr, Param, nullptr);
}

} // namespace tidy
} // namespace clang
} // namespace clang