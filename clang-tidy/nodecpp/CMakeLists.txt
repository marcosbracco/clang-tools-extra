set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyNodeCppModule
  NodeCppTidyModule.cpp
  NewNakedPtrCheck.cpp

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
