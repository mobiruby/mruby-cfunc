#include <stdint.h>
#include <stdio.h>

struct STest {
    int8_t x;
    int16_t y;
    int32_t z;
};

struct STest cfunc_test_func1(struct STest val) {
    val.z = val.x + val.y;
    return val;
}

struct STest2 {
    struct STest s;
    double xx;
};

struct STest2 cfunc_test_func2(struct STest2 val) {
    val.xx = (double)(val.s.x + val.s.y) / val.s.z;
    return val;
}

int cfunc_test_func3(int (*func)(uint32_t, uint32_t)) {
    return func(10, 20);
}

int cfunc_test_func4(int a, int b) {
    return a * b;
}

int check_offset5(uint8_t *p1, uint8_t *p2) {
    return (p1+5)==p2 ? 1 : 0;
}

static int test_func(uint32_t v1, uint32_t v2) { return v1 + v2; }

void test_func_ref() {
    struct STest v1 = {0, 0, 1};
    struct STest2 v2 = { {}, 0.0 };
    cfunc_test_func1(v1);
    cfunc_test_func2(v2);
    cfunc_test_func3(test_func);
    cfunc_test_func4(0, 1);
    uint8_t a, b;
    check_offset5(&a, &b);
}
