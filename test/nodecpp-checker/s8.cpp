// RUN: clang-tidy %s -- -std=c++11 -nostdinc++ | FileCheck %s -check-prefix=CHECK-MESSAGES -implicit-check-not="{{warning|error}}:"

 #include <cstring>

void func() {
	
	int i = 0;
	memset(&i,0,sizeof(i));
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: (S8)
}

