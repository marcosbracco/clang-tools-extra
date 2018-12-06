//===--- NakedStructCheck.cpp - clang-tidy---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NakedStructCheck.h"
#include "NakedPtrHelper.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace nodecpp {

void NakedStructCheck::registerMatchers(MatchFinder *Finder) {

}

void NakedStructCheck::check(const MatchFinder::MatchResult &Result) {

}

} // namespace nodecpp
} // namespace tidy
} // namespace clang
