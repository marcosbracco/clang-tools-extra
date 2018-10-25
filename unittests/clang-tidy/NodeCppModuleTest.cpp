#include "ClangTidyTest.h"
#include "gtest/gtest.h"

#include "nodecpp/ArrayTypeCheck.h"
#include "nodecpp/NakedPtrAssignmentCheck.h"
#include "nodecpp/NakedPtrFromFunctionCheck.h"
#include "nodecpp/NakedPtrFromMethodCheck.h"
#include "nodecpp/NakedPtrFuncCheck.h"
#include "nodecpp/NewExprCheck.h"
#include "nodecpp/NoCastCheck.h"
#include "nodecpp/PtrArithmeticCheck.h"
#include "nodecpp/StaticStorageCheck.h"
#include "nodecpp/VarDeclCheck.h"

using namespace clang::tidy::nodecpp;

namespace clang {
namespace tidy {
namespace test {

template <class T>
bool checkCode(const std::string &Code,
               const std::string &msg = std::string()) {
  std::vector<ClangTidyError> Errors;

  test::runCheckOnCode<T>(Code, &Errors);
  if (Errors.empty())
    return true;
  //  EXPECT_TRUE(Errors.size() == 1);
  if (Errors.size() != 1) {
    for (size_t i = 0; i != Errors.size(); ++i) {
      EXPECT_EQ(Errors[i].Message.Message, "");
    }
    EXPECT_TRUE(false);
  }
  EXPECT_TRUE(msg.empty() || Errors[0].Message.Message == msg);
  return false;
}

const std::string uniPtr = "namespace std {\n"
                     "    template<class T>\n"
                     "    class unique_ptr {\n"
                     "    public:\n"
                     "        unique_ptr(){}\n"
                     "        unique_ptr(T* ptr){}\n"
                     "        void reset(T* ptr){}\n"
                     "    };\n"
                     "}\n";


TEST(NodeCppModuleTest, ArrayTypeCheck) {
  EXPECT_FALSE(checkCode<nodecpp::ArrayTypeCheck>("int main() { int i[1]; }"));
}

TEST(NodeCppModuleTest, NakedPtrAssignmentCheck) {

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int main() { int* p1; int* p2; p2 = p1; }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int main() { int* p1; int* p2; p1 = p2; }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int main() { int* p1; { int* p2; p2 = p1; } }"));

  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int main() { int* p1; { int* p2; p1 = p2; } }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int main() { int p1; { int* p2; p2 = &p1; } }"));

  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int main() { int* p1; { int p2; p1 = &p2; } }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; func(p1); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int i; func(&i); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; int* p2 = func(p1); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int i; int* p2 = func(&i); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; int i; p1 = func(p1); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; int i; p1 = func(&i); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; { p1 = func(p1); } }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; int i; p1 = func(nullptr); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int* p = nullptr);"
      "int main() { int* p1; int i; p1 = func(); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "void other(int* arg) { int* p1; { p1 = func(arg); } }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; int* p2 = func(func(p1)); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; int i = *(func(p1)); }"));

  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*);"
      "int main() { int* p1; { int i; p1 = func(&i); } }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*, int*);" // here both args are ok
      "int main() { int* p1; int i; p1 = func(p1, &i); }"));

  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*, int*);" // worry about both args
      "int main() { int* p1; {int i; p1 = func(p1, &i); } }"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*, int*);" // worry about both args
      "int main() { int* p1; {int i; p1 = func(&i, p1); } }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int, int*);" // worry only about int*
      "int main() { int* p1; {int i; p1 = func(i, p1); } }"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int&);" // do worry about int&
      "int main() { int* p1; {int i; p1 = func(i); } }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int);" //don't worry about int as value
      "int main() { int* p1; {int i; p1 = func(i); } }"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some {};"
	  "int* func(Some&);" // do worry about Some&
      "int main() { int* p1; {Some s; p1 = func(s); } }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some {};"
      "Some* func(int&);" // int& can't be converted to Some*
      "int main() { Some* sp; {int i; sp = func(i); } }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "int* func(int*, char*);" //don't worry about char*
      "int main() { int* p1; { char* cp; p1 = func(p1, cp); } }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "char* func(char*, const char*);" //don't worry about const char*
      "int main() { char* p1; { const char* cp; p1 = func(p1, cp); } }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { int* get(); };"
      "int main() { Some s; s.get(); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { int* get(); };"
      "int main() { Some s; int* p2 = s.get(); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { int* get(); };"
      "int main() { Some s; int* p1; p1 = s.get(); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { int* get(); };"
      "int main() { Some s; Some* sp = &s; int* p1; p1 = sp->get(); }"));

  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { int* get(); };"
      "int main() { int* p1; { Some s; p1 = s.get(); } }"));

  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { Some* get(Some*); };"
      "int main() { Some* sp; Some s; sp = s.get(sp); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { Some* get(Some*); };"
      "void other(Some* arg) { Some* sp; Some s; sp = s.get(arg); }"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { Some* get(Some* sp = nullptr); };"
      "int main() { Some* sp; Some s; sp = s.get(); }"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { Some* get(Some*); };"
      "int main() { Some* sp; { Some s; sp = s.get(sp); } }"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrAssignmentCheck>(
      "struct Some { Some* get(Some*); };"
      "int main() { Some* sp; Some s; { Some* sp2; sp = s.get(sp2); } }"));
}

TEST(NodeCppModuleTest, NakedPtrFuncCheck) {
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrFuncCheck>("void good(int a);"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrFuncCheck>("void good(int& a);"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrFuncCheck>("void good(int* a);"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrFuncCheck>("int good();"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrFuncCheck>("int& good();"));
  EXPECT_TRUE(checkCode<nodecpp::NakedPtrFuncCheck>("int* good();"));
  EXPECT_TRUE(
      checkCode<nodecpp::NakedPtrFuncCheck>("struct Good { int good(); };"));
  EXPECT_TRUE(
      checkCode<nodecpp::NakedPtrFuncCheck>("struct Good { int& good(); };"));
  EXPECT_TRUE(
      checkCode<nodecpp::NakedPtrFuncCheck>("struct Good { int* good(); };"));

  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFuncCheck>("int*& bad();"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFuncCheck>("int** bad();"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFuncCheck>("void bad(int** a);"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFuncCheck>("void bad(int*& a);"));
  EXPECT_FALSE(
      checkCode<nodecpp::NakedPtrFuncCheck>("void bad(int* a, int** b);"));
  EXPECT_FALSE(
      checkCode<nodecpp::NakedPtrFuncCheck>("void bad(int* a, int*& b);"));
}

TEST(NodeCppModuleTest, NewExprCheckArray) {
  EXPECT_FALSE(checkCode<nodecpp::NewExprCheck>("int main() { new int[1]; }"));
}
TEST(NodeCppModuleTest, NewExprCheckPtr) {
  EXPECT_FALSE(checkCode<nodecpp::NewExprCheck>("int main() { new int*; }"));
}
TEST(NodeCppModuleTest, NewExprCheck) {
  EXPECT_FALSE(
      checkCode<nodecpp::NewExprCheck>("int main() { int* i = new int; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::NewExprCheck>("int main() { int* i = new int(0); }"));

  std::string bad = "class unique_ptr {\n"
                    "public:\n"
                    "    unique_ptr(){}\n"
                    "    unique_ptr(int *ptr){}\n"
                    "    void reset(int *ptr){}\n"
                    "};\n";
  EXPECT_FALSE(checkCode<nodecpp::NewExprCheck>(
      bad + "int main() { unique_ptr p(new int); }"));
  EXPECT_FALSE(checkCode<nodecpp::NewExprCheck>(
      bad + "int main() { unique_ptr p; p.reset(new int); }"));

  std::string good = "namespace std {\n"
                     "    template<class T>\n"
                     "    class unique_ptr {\n"
                     "    public:\n"
                     "        unique_ptr(){}\n"
                     "        unique_ptr(T* ptr){}\n"
                     "        void reset(T* ptr){}\n"
                     "    };\n"
                     "}\n";

  EXPECT_TRUE(checkCode<nodecpp::NewExprCheck>(
      uniPtr + "int main() { std::unique_ptr<int> p(new int); }"));
  EXPECT_TRUE(checkCode<nodecpp::NewExprCheck>(
      uniPtr +
      "using namespace std; int main() { unique_ptr<int> p(new int); }"));
  EXPECT_TRUE(checkCode<nodecpp::NewExprCheck>(
      uniPtr + "int main() { std::unique_ptr<int> p; p.reset(new int); }"));
}

TEST(NodeCppModuleTest, NoCastCheck) {
  EXPECT_FALSE(checkCode<nodecpp::NoCastCheck>(
      "int main() { size_t i; auto r = reinterpret_cast<void*>(i); }"));
  EXPECT_FALSE(checkCode<nodecpp::NoCastCheck>(
      "int main() { size_t i; auto r = (void*)i; }"));
  EXPECT_FALSE(checkCode<nodecpp::NoCastCheck>(
      "int main() { void* p; auto r = static_cast<size_t*>(p); }"));
  EXPECT_TRUE(checkCode<nodecpp::NoCastCheck>(
      "int main() { ((void) 0); }"));
  EXPECT_TRUE(checkCode<nodecpp::NoCastCheck>(
      "void func(int&& i) { int&& j = static_cast<int&&>(i); }"));
}

TEST(NodeCppModuleTest, PtrArithmeticCheck) {
  EXPECT_FALSE(checkCode<nodecpp::PtrArithmeticCheck>(
      "int main() { int* a; a = a + 1; }"));
  EXPECT_FALSE(checkCode<nodecpp::PtrArithmeticCheck>(
      "int main() { int* a; a = a - 1; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; a += 1; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; a -= 1; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; a++; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; a--; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; ++a; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; --a; }"));
  EXPECT_FALSE(checkCode<nodecpp::PtrArithmeticCheck>(
      "int main() { int* a; int b = a[1]; }"));
}

TEST(NodeCppModuleTest, StaticStorageCheck) {
  EXPECT_TRUE(
      checkCode<nodecpp::StaticStorageCheck>("constexpr int good = 5;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("int bad;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("extern int bad;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("static int bad;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("thread_local int bad;"));

  EXPECT_FALSE(
      checkCode<nodecpp::StaticStorageCheck>("int main() { static int bad; }"));

  EXPECT_TRUE(checkCode<nodecpp::StaticStorageCheck>(
      "class Good { static constexpr int good = 5; };"));
  EXPECT_FALSE(
      checkCode<nodecpp::StaticStorageCheck>("class Bad { static int bad; };"));
}

TEST(NodeCppModuleTest, VarDeclCheck) {
  EXPECT_TRUE(checkCode<nodecpp::VarDeclCheck>("struct Safe1 { int i; };"
                                               "Safe1 s;"));
  EXPECT_TRUE(checkCode<nodecpp::VarDeclCheck>(
      uniPtr + "struct Safe1 { int i; };"
               "struct Safe2 { std::unique_ptr<Safe1> ptr; Safe1 s; };"
               "Safe2 s;"));

  EXPECT_TRUE(checkCode<nodecpp::VarDeclCheck>(
      uniPtr + "struct Safe1 { int i; };"
               "struct Safe2 { std::unique_ptr<Safe1> ptr; Safe1 s; };"
               "std::unique_ptr<Safe2> ptr;"));

  EXPECT_TRUE(
      checkCode<nodecpp::VarDeclCheck>("struct Safe1 { int i; };"
                                       "struct Safe2 : public Safe1 { };"
                                       "Safe2 s;"));

  EXPECT_TRUE(checkCode<nodecpp::VarDeclCheck>("struct NakedStr { int* ptr; };"
                                               "NakedStr naked;"));

  EXPECT_TRUE(checkCode<nodecpp::VarDeclCheck>(
      "struct NakedStr { int* ptr; int* get() const; };"
      "NakedStr naked;"));

  EXPECT_TRUE(checkCode<nodecpp::VarDeclCheck>("int* ptr;"));

  EXPECT_FALSE(checkCode<nodecpp::VarDeclCheck>("int** ptr;"));

  EXPECT_FALSE(checkCode<nodecpp::VarDeclCheck>(
      "struct NakedStr { int* ptr; int* get() const; };"
      "NakedStr* naked;"));

  EXPECT_FALSE(checkCode<nodecpp::VarDeclCheck>(
      "struct Bad { int* ptr; void set(int* ptr); };"
      "Bad bad;"));
}

} // namespace test
} // namespace tidy
} // namespace clang
