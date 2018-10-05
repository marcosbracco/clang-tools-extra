// RUN: %check_clang_tidy %s nodecpp-naked-ptr-from-method %t


struct Some {
	int* get();
	Some* join(Some* other = nullptr);
};

void f(Some* arg) {
	Some s;

	s.get(); //ok
	int* p2 = s.get(); //ok

	int* p1;
	p1 = s.get(); //ok

	Some* sp = &s;
	p1 = sp->get(); //ok

	{
		Some sInt;
		p1 = sInt.get(); //bad instance goes out of scope
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-from-method]
	}

	sp = s.join(sp); //ok
	sp = s.join(arg); //ok
	sp = s.join();	//ok

	{
		Some sInt;
		sp = sInt.join(sp); //bad instance goes out of scope
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-from-method]
	}

	{
		Some* ptrInt;
		sp = s.join(ptrInt); //bad argument goes out of scope
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-from-method]
	}
}

