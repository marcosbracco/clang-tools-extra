// RUN: %check_clang_tidy %s nodecpp-naked-ptr-return %t




int* bad1() {
    int i;
    return &i;
// CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'f' is insufficiently awesome [nodecpp-static-storage]
}

int* good1(int* a) {
    return a;
}

int* good2(bool cond, int* a, int* b) {
    if(cond)
        return a;
    else
        return b;
}

int* good3(bool cond, int* a, int* b) {
    return cond ? a : b;
}

class Safe {
    int a;

    int* good() {
        return &a;
    }
};

