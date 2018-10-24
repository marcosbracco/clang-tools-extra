// RUN: %check_clang_tidy %s nodecpp-var-decl %t

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
	int* ptr;

	int* get() const;
	NakedStr();
	NakedStr(const NakedStr&);
	NakedStr& operator=(const NakedStr&) = delete;
};

void nakedFunc() {
	
	int* i; //ok
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
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]
	NakedStr* nakedPtr; // bad
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]

	Bad1 b1; //bad
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]
	b1 = Bad1(); //this forces Bad1::operator= to be instantiated
	Bad2 b2; //bad
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]
	Bad3 b3; //bad
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]
}
