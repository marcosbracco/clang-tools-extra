// RUN: clang-tidy %s --checks=-*,nodecpp-raw-pointer-dereference -- -std=c++11 -nostdinc++ -isystem %S/Inputs | FileCheck %s -check-prefix=CHECK-MESSAGES -implicit-check-not="{{warning|error}}:"

#include <safe_ptr.h>

using namespace nodecpp;


struct Some {
    int i = 0;
};

void f() {
    int i;
    *(&i);
// CHECK-MESSAGES: :[[@LINE-1]]:5: warning: do not dereference raw pointer [nodecpp-raw-pointer-dereference]
    int* p = nullptr;
    *p;
// CHECK-MESSAGES: :[[@LINE-1]]:5: warning: do not dereference raw pointer [nodecpp-raw-pointer-dereference]

    Some s;
    (&s)->i;
// CHECK-MESSAGES: :[[@LINE-1]]:11: warning: do not dereference raw pointer [nodecpp-raw-pointer-dereference]

    owning_ptr<Some> u;
    u->i; //ok
    u.get()->i; //ok
    *u; //ok
}

class Good {
	int i;
	int get() const {
		return this->i;//ok
	}
};
