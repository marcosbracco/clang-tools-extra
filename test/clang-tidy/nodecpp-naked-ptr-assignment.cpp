// RUN: %check_clang_tidy %s nodecpp-naked-ptr-assignment %t

void good1() { 
	int* p1; 
	int* p2; 
	p2 = p1; 
}

void good2() { 
	int* p1; 
	{ 
		int* p2; 
		p2 = p1;
	}
}

void bad1() {
	int* p1;
	{
		int* bad;
		p1 = bad;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: assignment of naked pointer may extend scope [nodecpp-naked-ptr-assignment]
	} 
}

void good3() {
	int p1;
	{
		int* p2;
		p2 = &p1;
	}
}

void bad2() {
	int* p1;
	{
		int bad;
		p1 = &bad;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: assignment of naked pointer may extend scope [nodecpp-naked-ptr-assignment]
	}
}

