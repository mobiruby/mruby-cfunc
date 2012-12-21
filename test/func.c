struct STest {
    int8_t x;
    int16_t y;
    int32_t z;
};

struct STest cfunc_test_func1(struct STest val) {
    val.z = val.x + val.y;
    return val;
};

struct STest2 {
    struct STest s;
    double xx;
};

struct STest2 cfunc_test_func2(struct STest2 val) {
    val.xx = (double)(val.s.x + val.s.y) / val.s.z;
    return val;
};

int cfunc_test_func3(int (*func)(int, int)) {
    return func(10, 20);
}

int cfunc_test_func4(int a, int b) {
    return a * b;
}

int check_offset5(uint8_t *p1, uint8_t *p2) {
    return (p1+5)==p2 ? 1 : 0;
}
