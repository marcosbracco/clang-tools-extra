#include "ClangTidyTest.h"
#include "gtest/gtest.h"

#include "nodecpp/ArrayTypeCheck.h"
#include "nodecpp/NakedPtrFieldCheck.h"
#include "nodecpp/NakedPtrFuncCheck.h"
#include "nodecpp/NewExprCheck.h"
#include "nodecpp/PtrArithmeticCheck.h"
#include "nodecpp/StaticStorageCheck.h"

using namespace clang::tidy::nodecpp;

namespace clang {
namespace tidy {
namespace test {


template<class T>
bool checkCode(const std::string &Code, const std::string& msg = std::string()) {
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

TEST(NodeCppModuleTest, ArrayTypeCheck) {
  EXPECT_FALSE(checkCode<nodecpp::ArrayTypeCheck>("int main() { int i[1]; }"));
}

TEST(NodeCppModuleTest, NakedPtrFieldCheck) {
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFieldCheck>("class Bad { int* i; };"));
}

TEST(NodeCppModuleTest, NakedPtrFuncCheck) {
  EXPECT_TRUE(
      checkCode<nodecpp::NakedPtrFuncCheck>("int good(int* a) { return *a; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::NakedPtrFuncCheck>(
      "int* bad() { return new int; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::NakedPtrFuncCheck>("class Bad { int i; int* bad() { return &i; } };"));

  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFuncCheck>(
      "int*& bad(int* a) { static int* i = new int; return i; }"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFuncCheck>(
      "void bad(int* a, int** b) { b = &a; }"));
  EXPECT_FALSE(checkCode<nodecpp::NakedPtrFuncCheck>(
      "void bad(int* a, int*& b) { b = a; }"));
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
  EXPECT_FALSE(checkCode<nodecpp::NewExprCheck>(bad + 
	  "int main() { unique_ptr p(new int); }"));
  EXPECT_FALSE(checkCode<nodecpp::NewExprCheck>(
      bad + "int main() { unique_ptr p; p.reset(new int); }"));

  std::string good = 
				    "namespace nodecpp {\n"
                     "    template<class T>\n"
                    "    class unique_ptr {\n"
                    "    public:\n"
                    "        unique_ptr(){}\n"
                    "        unique_ptr(T* ptr){}\n"
                    "        void reset(T* ptr){}\n"
                    "    };\n"
					"}\n";

  EXPECT_TRUE(checkCode<nodecpp::NewExprCheck>(
      good + "int main() { nodecpp::unique_ptr<int> p(new int); }"));
  EXPECT_TRUE(checkCode<nodecpp::NewExprCheck>(
      good + "using namespace nodecpp; int main() { unique_ptr<int> p(new int); }"));
  EXPECT_TRUE(checkCode<nodecpp::NewExprCheck>(
      good + "int main() { nodecpp::unique_ptr<int> p; p.reset(new int); }"));
}

TEST(NodeCppModuleTest, PtrArithmeticCheck) {
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; a = a + 1; }"));
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; a = a - 1; }"));
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
  EXPECT_FALSE(
      checkCode<nodecpp::PtrArithmeticCheck>("int main() { int* a; int b = a[1]; }"));
}

TEST(NodeCppModuleTest, StaticStorageCheck) {
  EXPECT_TRUE(checkCode<nodecpp::StaticStorageCheck>("constexpr int good = 5;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("int bad;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("extern int bad;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("static int bad;"));
  EXPECT_FALSE(checkCode<nodecpp::StaticStorageCheck>("thread_local int bad;"));

  EXPECT_FALSE(
      checkCode<nodecpp::StaticStorageCheck>("int main() { static int bad; }"));

  EXPECT_TRUE(
      checkCode<nodecpp::StaticStorageCheck>("class Good { static constexpr int good = 5; };"));
  EXPECT_FALSE(
      checkCode<nodecpp::StaticStorageCheck>("class Bad { static int bad; };"));
}



} // namespace test
} // namespace tidy
} // namespace clang
