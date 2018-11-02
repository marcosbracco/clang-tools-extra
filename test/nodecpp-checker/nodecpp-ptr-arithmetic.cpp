// RUN: %check_nodecpp_checker %s nodecpp-ptr-arithmetic %t


void bad() {
	int* a;

	a = a + 1;
// CHECK-MESSAGES: :[[@LINE-1]]:8: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	a = a - 1;
// CHECK-MESSAGES: :[[@LINE-1]]:8: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	a += 1;
// CHECK-MESSAGES: :[[@LINE-1]]:4: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	a -= 1;
// CHECK-MESSAGES: :[[@LINE-1]]:4: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	a++;
// CHECK-MESSAGES: :[[@LINE-1]]:3: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	++a;
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	a--;
// CHECK-MESSAGES: :[[@LINE-1]]:3: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	--a;
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: do not use pointer arithmetic [nodecpp-ptr-arithmetic]
	int b = a[0];
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: do not use index operator on unsafe types [nodecpp-ptr-arithmetic]
}
