// RUN: %check_clang_tidy %s nodecpp-naked-ptr-func %t

void good1(int a);
void good2(int& a);
void good3(int* a);
int good4();
int& good5();
int* good6();

struct Good1 { 
	int good();
};

struct Good2 {
	int& good();
};

struct Good3 {
	int* good();
};

int*& bad1();
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-func]
int** bad2();
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-func]
void bad3(int** a);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-func]
void bad4(int*& a);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-func]
void bad5(int* a, int** b);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-func]
void bad6(int* a, int*& b);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-func]
