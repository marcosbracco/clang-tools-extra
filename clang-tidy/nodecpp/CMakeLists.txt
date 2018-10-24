set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyNodeCppModule
  MayExtendLambdaCheck.cpp
  NakedPtrFromOperatorCheck.cpp
  NakedPtrHelper.cpp
  ArrayTypeCheck.cpp
  NakedPtrAssignmentCheck.cpp
  NakedPtrFromFunctionCheck.cpp
  NakedPtrFromMethodCheck.cpp
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
