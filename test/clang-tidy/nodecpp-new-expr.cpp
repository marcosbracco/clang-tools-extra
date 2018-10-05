// RUN: %check_clang_tidy %s nodecpp-new-expr %t


// good definition of nodecpp::unique_ptr
namespace nodecpp {
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
	new int[1];
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-new-expr]
}

void bad2() {
 new int*;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-new-expr]
}

void bad3() {
	int* i = new int;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-new-expr]
}

void bad4() {
	int* i = new int(0);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-new-expr]
}

void bad5() {
	unique_ptr p(new int);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-new-expr]
}

void bad6() {
	unique_ptr p; 
	p.reset(new int);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-new-expr]
}


void good1() {
	nodecpp::unique_ptr<int> p(new int);
}

void good2() {
	nodecpp::unique_ptr<int> p;
	p.reset(new int);
}

using namespace nodecpp;
void good3() {
	unique_ptr<int> p(new int);
}
 
