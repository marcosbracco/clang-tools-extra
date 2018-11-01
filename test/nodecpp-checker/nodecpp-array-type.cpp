// RUN: %check_clang_tidy %s nodecpp-array-type %t

void f() { 
	int i[1];
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-array-type]
}
