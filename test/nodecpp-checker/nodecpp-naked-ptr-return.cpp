// RUN: %check_nodecpp_checker %s nodecpp-naked-ptr-return %t




int* bad1() {
    int i;
    return &i; //bad return address of local var
// CHECK-MESSAGES: :[[@LINE-1]]:12: warning: return of naked pointer may extend scope [nodecpp-naked-ptr-return]
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

