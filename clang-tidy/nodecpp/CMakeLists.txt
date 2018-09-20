set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyNodeCppModule
  ArrayTypeCheck.cpp
  NakedPtrAssignmentCheck.cpp
  NakedPtrFieldCheck.cpp
  NakedPtrFromFunctionCheck.cpp
  NakedPtrFromMethodCheck.cpp
  NakedPtrFuncCheck.cpp
  NewExprCheck.cpp
  NoCastCheck.cpp
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
