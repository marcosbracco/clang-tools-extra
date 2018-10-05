// RUN: %check_clang_tidy %s nodecpp-static-storage %t


constexpr int good = 5;

int bad1;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]

extern int bad2;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]

static int bad3;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]

thread_local int bad4;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]

void func() {
	static int bad;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]
}

class Good {
	static constexpr int good = 5;
};


class Bad {
	static int bad;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]
};
