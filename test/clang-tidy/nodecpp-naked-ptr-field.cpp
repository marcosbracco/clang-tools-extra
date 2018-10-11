// RUN: %check_clang_tidy %s nodecpp-naked-ptr-field %t


namespace nodecpp {
template<class T>
class unique_ptr {
	T* t;
};
}

template<class T>
class bad_ptr {
	T* t;
};

class Bad1 {
	int* i;
};
    
void bad1() {
	Bad1 b; //ok now 
}

void good1() {
	nodecpp::unique_ptr<int> i;
}

class Good {
	nodecpp::unique_ptr<int> i;
};

void good2() {
	Good g;
}

void bad2() {
	bad_ptr<int> i; //ok now
}

class Bad3 {
	bad_ptr<int> i;
};

void bad3() {
	Bad3 b; //ok now
}

