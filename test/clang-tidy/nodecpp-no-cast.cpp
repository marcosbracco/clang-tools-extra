// RUN: %check_clang_tidy %s nodecpp-no-cast %t


void bad1() { 
	size_t i;
	auto r = reinterpret_cast<void*>(i);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-no-cast]
}

void bad2() {
	size_t i;
	auto r = (void*)i;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-no-cast]
}

void bad3() {
	void* p;
	auto r = static_cast<size_t*>(p);
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-no-cast]
}

void good1() {
	((void) 0); //ok, definition of assert macro

}
