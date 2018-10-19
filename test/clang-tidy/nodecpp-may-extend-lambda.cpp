// RUN: %check_clang_tidy %s nodecpp-may-extend-lambda %t


#include <functional>

struct Infra {
	void on([[nodecpp::may_extend]] std::function<void()> func);
};

struct [[nodecpp::no_instance]] Sock {
};

struct Bad {
};

void plainFunc() {}


struct [[nodecpp::no_instance]] SafeCode {
	Infra infra;

	void aMethod(Sock* sock) {}
	void other(Bad* bad) {}


	void callback(Sock* sock) {
		infra.on([this, sock]() { this->aMethod(sock); }); // ok, 'this' and 'sock' are safe
// CHECK-MESSAGES: :[[@LINE-1]]:33: warning: capture type not allowed [nodecpp-may-extend-lambda]
		infra.on([&]() {this->aMethod(sock); }); //bad, 'sock' by reference 
// CHECK-MESSAGES: :[[@LINE-1]]:19: warning: capture pointer type not allowed [nodecpp-may-extend-lambda]

		Bad* badPtr = nullptr;
		infra.on([this, badPtr]() { this->other(badPtr);} ); //bad, 'badPtr' not safe 
// CHECK-MESSAGES: :[[@LINE-1]]:19: warning: capture pointer type not allowed [nodecpp-may-extend-lambda]

		auto l = [this, badPtr]() { this->other(badPtr);}; //bad, 'badPtr' not safe
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: failed to verify argument [nodecpp-may-extend-lambda]
		infra.on(l);		

		infra.on(&plainFunc); //bad, only lambdas allowed
	}
};

struct NonSafe {
	Infra infra;

	void aMethod(Sock* sock) {}
	void other(Bad* bad) {}


	void callback(Sock* sock) {
		infra.on([this, sock]() { this->aMethod(sock); }); //bad, 'this' not safe
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: capture pointer type not allowed [nodecpp-may-extend-lambda]

		auto l = [this, sock]() { this->aMethod(sock); }; //bad, 'this' not safe
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: capture pointer type not allowed [nodecpp-may-extend-lambda]
		infra.on(l); 
	}
};

