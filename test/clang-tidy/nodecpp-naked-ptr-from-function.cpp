// RUN: %check_clang_tidy %s nodecpp-naked-ptr-from-function %t

int* func(int* p = nullptr);
int* func2(int* p1, int* p2);
int* func3(int, int*); // worry only about int*
int* func4(int&);

struct Some {};
int* func5(Some&);

Some* func6(int&, Some*); //int& can't become Some*

int* func7(int*, char*); //don't worry about char*

char* func8(char*, const char*); //don't worry about const char*

void f(int* arg) {
	int* p1;
	int i;

	func(p1); //ok
	func(&i); //ok

	int* p2 = func(p1); //ok
	int* p3 = func(&i); //ok

	p1 = func(p1); //ok
	p1 = func(&i); //ok

	{	
		p1 = func(p2); //ok p1 and p2 have same life
	}

	p1 = func(nullptr); //ok
	p1 = func(); //ok default arg used

	{
		p1 = func(arg); // argument are ok
	}

	int* goodp = func(func(p1)); 

	int good = *(func(p1));


	{
		int iBad;
		p1 = func(&iBad); //bad
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: assignment of naked pointer may extend scope [nodecpp-naked-ptr-assignment]
	}

	p1 = func2(p1, &i); // both args ok

	{	
		int iBad;
		p1 = func2(p1, &iBad); // bad
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: assignment of naked pointer may extend scope [nodecpp-naked-ptr-assignment]
	}

	{	
		int iBad;
		p1 = func2(&iBad, p1); //bad 
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: assignment of naked pointer may extend scope [nodecpp-naked-ptr-assignment]
	}
	{
		int i; 
		p1 = func3(i, p1); //ok, don't worry about value arg
	}

	{
		int i;
		p1 = func4(i); //bad, worry about ref arg
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: assignment of naked pointer may extend scope [nodecpp-naked-ptr-assignment]
	}

	{
		Some s;
		p1 = func5(s); //bad, assume Some can return int*
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: assignment of naked pointer may extend scope [nodecpp-naked-ptr-assignment]
	}

	Some* sp;
	{
		int i;
		sp = func6(i, sp); //ok
	}

	{
		char* cp;
		p1 = func7(p1, cp); //ok, char* can't become int*
	}

	char* cp1;
	{
		const char* cp2;
		cp1 = func8(cp1, cp2); // ok conts char can't become char*
	}
}

