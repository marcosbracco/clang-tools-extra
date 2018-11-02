// RUN: %check_nodecpp_checker %s nodecpp-no-cast %t


void bad1() { 
	size_t i;
	auto r1 = reinterpret_cast<void*>(i);
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: do not use cast [nodecpp-no-cast]
	auto r2 = (void*)i;
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: do not use cast [nodecpp-no-cast]
	void* p;
	auto r3 = static_cast<size_t*>(p);
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: do not use cast [nodecpp-no-cast]

}

void good1(int&& i) {
	((void) 0); //ok, definition of assert macro

	int&& j = static_cast<int&&>(i); //static cast to rvalue is ok
}
