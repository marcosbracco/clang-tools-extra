set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyNodeCppModule
  ArrayTypeCheck.cpp
  NakedPtrFieldCheck.cpp
  NakedPtrFuncCheck.cpp
  NewArrayExprCheck.cpp
  NewExprCheck.cpp
  NodeCppTidyModule.cpp
  PtrArithmeticCheck.cpp
  StaticStorageCheck.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTidy
  clangTidyMiscModule
  clangTidyUtils
  clangTooling
  )
