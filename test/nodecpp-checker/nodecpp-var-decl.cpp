// RUN: clang-tidy %s --checks=-*,nodecpp-var-decl -- -std=c++11 -nostdinc++ | FileCheck %s -check-prefix=CHECK-MESSAGES -implicit-check-not="{{warning|error}}:"

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

struct Safe1 {
	int i;
};

struct Safe2 {
	std::unique_ptr<Safe1> s1Ptr;

	Safe1 s1;
};

class Safe3 :public Safe2 {};

void safeFun() {
	//all ok
	Safe1 s1;

	Safe2 s2;
	std::unique_ptr<Safe2> s2Ptr;

	Safe3 s3;
	std::unique_ptr<Safe3> s3Ptr;
}


struct NakedStr {
	int* ptr = nullptr;

	int* get() const;
	NakedStr();
	NakedStr(const NakedStr&);
	NakedStr& operator=(const NakedStr&) = delete;
};

void nakedFunc() {
	
	int* i = nullptr; //ok
	NakedStr naked; //ok
}

struct Bad1 {
	int* ptr;
};

struct Bad2 : public NakedStr {

};

struct Bad3 {
	int* ptr;

	void set(int* ptr);
};

void badFunc() {
	int** i; //bad
// CHECK-MESSAGES: :[[@LINE-1]]:8: warning: unsafe type at variable declaration [nodecpp-var-decl]
	NakedStr* nakedPtr; // bad
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: unsafe type at variable declaration [nodecpp-var-decl]

	Bad1 b1; //bad
// CHECK-MESSAGES: :[[@LINE-1]]:7: warning: unsafe type at variable declaration [nodecpp-var-decl]
	b1 = Bad1(); //this forces Bad1::operator= to be instantiated
	Bad2 b2; //bad
// CHECK-MESSAGES: :[[@LINE-1]]:7: warning: unsafe type at variable declaration [nodecpp-var-decl]
	Bad3 b3; //bad
// CHECK-MESSAGES: :[[@LINE-1]]:7: warning: unsafe type at variable declaration [nodecpp-var-decl]
}

class Sock {};

class Safe {

	void mayExtendCallback(Sock* dontExtend, Sock* sock [[nodecpp::may_extend_to_this]]) {
		Sock* other [[nodecpp::may_extend_to_this]] = sock;
		Sock* other2 [[nodecpp::may_extend_to_this]] = dontExtend; //bad donExtend is not valid initializer
// CHECK-MESSAGES: :[[@LINE-1]]:9: warning: initializer not allowed to may_extend declaration [nodecpp-var-decl]
	}

};
