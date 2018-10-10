// RUN: %check_clang_tidy %s nodecpp-naked-ptr-from-operator %t



struct Some {
	long* operator>>(long* p) { return p; }
};

int* operator>>(Some& s, int* a) { return a; }

void f() {


	int* p1 = nullptr;
	long* lp = nullptr;
	Some s;
	{
		int i = 0;
		long l = 0;

		auto f = [&](int* p, long) { return p; };

		p1 = f(p1, l); //ok

		p1 = f(&i, l); // bad i goes out of scope
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-from-method]




		p1 = (s >> p1); //ok function op

		p1 = (s >> &i); // bad function op
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-from-method]

		lp = s >> lp; // ok method op

		lp = s >> &l; // bad method op
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-naked-ptr-from-method]

	}
}
