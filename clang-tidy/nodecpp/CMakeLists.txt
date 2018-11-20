set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyNodeCppModule
  ConstCheck.cpp
  MayExtendDeclCheck.cpp
  MayExtendLambdaCheck.cpp
  NakedAssignmentCheck.cpp
  NakedPtrHelper.cpp
  ArrayTypeCheck.cpp
  NakedPtrFuncCheck.cpp
  NakedPtrReturnCheck.cpp
  NakedStructCheck.cpp
  NewExprCheck.cpp
  NoCastCheck.cpp
  NodeCppTidyModule.cpp
  PtrArithmeticCheck.cpp
  RawPointerAssignmentCheck.cpp
  RawPointerDereferenceCheck.cpp
  StaticStorageCheck.cpp
  StdFunctionCheck.cpp
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
