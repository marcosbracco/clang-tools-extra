// RUN: clang-tidy %s -- -std=c++11 -nostdinc++ | FileCheck %s -check-prefix=CHECK-MESSAGES -implicit-check-not="{{warning|error}}:"


union Bad {
	int i;
	int* ptr;
// CHECK-MESSAGES: :[[@LINE-1]]:7: warning: (S1.4)
};

void func1() { 
	Bad bad;
// CHECK-MESSAGES: :[[@LINE-1]]:6: note:
}


template <class T>
union MayBeBad {
	int i;
	T ptr;
// CHECK-MESSAGES: :[[@LINE-1]]:4: warning: (S1.4)
};

void func2() { 

	MayBeBad<long> good; //ok
	MayBeBad<long*> bad;
// CHECK-MESSAGES: :[[@LINE-1]]:18: note:
}

class UsesUnion {
	MayBeBad<int*> pt;
};

void func3() { 

	UsesUnion uu;
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: unsafe type at variable declaration [nodecpp-var-decl]
}



