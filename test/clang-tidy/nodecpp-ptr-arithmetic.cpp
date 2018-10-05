// RUN: %check_clang_tidy %s nodecpp-ptr-arithmetic %t


void bad() {
	int* a;

	a = a + 1;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	a = a - 1;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	a += 1;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	a -= 1;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	a++;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	++a;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	a--;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	--a;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
	int b = a[0];
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-ptr-arithmetic]
}
