set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyNodeCppModule
  MayExtendLambdaCheck.cpp
  NakedPtrHelper.cpp
  ArrayTypeCheck.cpp
  NakedPtrAssignmentCheck.cpp
  NakedPtrFuncCheck.cpp
  NewExprCheck.cpp
  NoCastCheck.cpp
  NodeCppTidyModule.cpp
  PtrArithmeticCheck.cpp
  StaticStorageCheck.cpp
  VarDeclCheck.cpp

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
