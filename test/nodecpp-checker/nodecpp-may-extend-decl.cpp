// RUN: %check_clang_tidy %s nodecpp-may-extend-decl %t

class Sock {};

void f() {

    auto l = [](Sock* s [[nodecpp::may_extend_to_this]]) {}; //bad attribute not valid at lambda    
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-may-extend-decl]
}
