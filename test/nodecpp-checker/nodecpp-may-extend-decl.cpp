// RUN: %check_nodecpp_checker %s nodecpp-may-extend-decl %t

class Sock {};

void f() {

    auto l = [](Sock* s [[nodecpp::may_extend_to_this]]) {}; //bad attribute not valid at lambda    
// CHECK-MESSAGES: :[[@LINE-1]]:23: warning: attribute [may_extend_to_this] can't be used on lambdas [nodecpp-may-extend-decl]
}
