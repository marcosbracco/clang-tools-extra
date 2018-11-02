// RUN: %check_clang_tidy %s nodecpp-array-type %t

void f() { 
	int i[1];
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: do not use arrays [nodecpp-array-type]
}
