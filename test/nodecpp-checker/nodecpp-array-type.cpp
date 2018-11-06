// RUN: clang-tidy %s --checks=-*,nodecpp-array-type -- -std=c++11 -nostdinc++ | FileCheck %s -check-prefix=CHECK-MESSAGES -implicit-check-not="{{warning|error}}:"

void f() { 
	int i[1];
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: do not use arrays [nodecpp-array-type]
}
