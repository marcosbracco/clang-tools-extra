// RUN: %check_clang_tidy %s nodecpp-static-storage %t


constexpr int good = 5;

int bad1;
// CHECK-MESSAGES: :[[@LINE-1]]:5: warning: do not use global, static or thread_local variables [nodecpp-static-storage]

extern int bad2;
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: do not use global, static or thread_local variables [nodecpp-static-storage]

static int bad3;
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: do not use global, static or thread_local variables [nodecpp-static-storage]

thread_local int bad4;
// CHECK-MESSAGES: :[[@LINE-1]]:18: warning: do not use global, static or thread_local variables [nodecpp-static-storage]

void func() {
	static int bad;
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: do not use global, static or thread_local variables [nodecpp-static-storage]
}

class Good {
	static constexpr int good = 5;
};


class Bad {
	static int bad;
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: do not use global, static or thread_local variables [nodecpp-static-storage]
};
