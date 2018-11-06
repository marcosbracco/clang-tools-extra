// RUN: clang-tidy %s --checks=-*,nodecpp-new-expr -- -std=c++11 -nostdinc++ | FileCheck %s -check-prefix=CHECK-MESSAGES -implicit-check-not="{{warning|error}}:"


// good definition of nodecpp::unique_ptr
namespace std {
	template<class T>
	class unique_ptr {
	public:
		unique_ptr() {}
		unique_ptr(T* ptr) {}
		void reset(T* ptr) {}
	};
}

//bad definition, not template, no namespace
class unique_ptr {
public:
	unique_ptr() {}
	unique_ptr(int *ptr) {}
	void reset(int *ptr) {}
};
  

void bad1() {
	new int[1]; //bad, array
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: do not use array new expression [nodecpp-new-expr]
}

void bad2() {
 new int*; //bad, pointer
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: type is not safe for heap allocation [nodecpp-new-expr]
}

void bad3() {
	int* i = new int; //bad, not owner
// CHECK-MESSAGES: :[[@LINE-1]]:11: warning: new expresion must be owned by a unique_ptr [nodecpp-new-expr]
}

void bad4() {
	int* i = new int(0); //bad, not owner
// CHECK-MESSAGES: :[[@LINE-1]]:11: warning: new expresion must be owned by a unique_ptr [nodecpp-new-expr]
}

void bad5() {
	unique_ptr p(new int); //bad, onwer class is not nodecpp::unique_ptr
// CHECK-MESSAGES: :[[@LINE-1]]:15: warning: new expresion must be owned by a unique_ptr [nodecpp-new-expr]
}

void bad6() {
	unique_ptr p; 
	p.reset(new int); //bad, onwer class is not nodecpp::unique_ptr
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: new expresion must be owned by a unique_ptr [nodecpp-new-expr]
}


void good1() {
	std::unique_ptr<int> p(new int);
}

void good2() {
	std::unique_ptr<int> p;
	p.reset(new int);
}

using namespace std;
void good3() {
	unique_ptr<int> p(new int);
}
 
