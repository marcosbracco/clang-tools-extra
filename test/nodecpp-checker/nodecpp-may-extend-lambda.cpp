// RUN: %check_nodecpp_checker %s nodecpp-may-extend-lambda %t


namespace std {
	template< class R, class... Args >
	class function {
		public:
		template<class T>
		function(T t) {}
	};
}

struct Sock {};
struct Bad {};

void plainFunc() {}

struct Infra {
	void plain(std::function<void()> func);

	void onMayExtend(std::function<void()> func [[nodecpp::may_extend_to_this]]);
};


struct SafeCode {
	Infra infra;

	void aMethod() {}
	void aMethod(Sock* sock) {}
	void aMethod(Bad* bad) {}

	void m() {
		int* p = nullptr;

		infra.onMayExtend([this]() { this->aMethod(); }); // ok,

		Bad* badPtr = nullptr;
		infra.onMayExtend([this, badPtr]() { this->aMethod(badPtr);} ); //bad, 'badPtr' not safe 
// CHECK-MESSAGES: :[[@LINE-1]]:28: warning: unsafe capture to extend scope [nodecpp-may-extend-lambda]

		auto la = [this, badPtr]() { this->aMethod(badPtr);}; //ok, used without may_extend
		infra.plain(la);		

		auto l = [this, badPtr]() { this->aMethod(badPtr);}; //bad, 'badPtr' used with may_extend
// CHECK-MESSAGES: :[[@LINE-1]]:19: warning: unsafe capture to extend scope [nodecpp-may-extend-lambda]
		infra.onMayExtend(l);		
// CHECK-MESSAGES: :[[@LINE-1]]:21: note: referenced from here

		infra.onMayExtend(plainFunc); //ok function poninters always ok

	}

	void callback(Sock* sock) {

		infra.onMayExtend([this, sock]() { this->aMethod(sock); }); // bad, 'sock is not safe to may_extend
// CHECK-MESSAGES: :[[@LINE-1]]:28: warning: unsafe capture to extend scope [nodecpp-may-extend-lambda]
	}


	void mayExtendCallback(Sock* dontExtend, Sock* sock [[nodecpp::may_extend_to_this]]) {
		infra.onMayExtend([this, sock]() { this->aMethod(sock); }); // ok, now 'sock is safe

		Sock* other [[nodecpp::may_extend_to_this]] = sock;
		infra.onMayExtend([this, other]() { this->aMethod(other); }); // ok, 'other' is also safe

		Sock* other2 [[nodecpp::may_extend_to_this]] = dontExtend; //bad donExtend is not valid initializer
	}

};

struct Bad2 {
	void bad(Infra infra) {
		infra.onMayExtend([this]() { });// bad, infra is not a member
// CHECK-MESSAGES: :[[@LINE-1]]:9: warning: methods with [may_extend_to_this] attribute can be called only on members that share the lifetime of this [nodecpp-may-extend-lambda]
	}
};


class Infra2 {
public:
	void on(std::function<void(Sock* sock)> f [[nodecpp::may_extend_to_this]]);
};

void func(std::function<void(Sock* sock)> f);

class Safe {
	Infra2 infra;
	void m1() {
		int i;
		auto l = [this, &i](Sock* sock) {};
// CHECK-MESSAGES: :[[@LINE-1]]:20: warning: unsafe capture to extend scope [nodecpp-may-extend-lambda]
		infra.on(l);
// CHECK-MESSAGES: :[[@LINE-1]]:12: note: referenced from here
	} 

	void m2() {
		int i;
		auto l = [this, i](Sock* sock) {};
		infra.on(l);
	}

	void m3() {
		int i;
		auto l = [this, &i](Sock* sock) {};
		func(l);
	} 
};

