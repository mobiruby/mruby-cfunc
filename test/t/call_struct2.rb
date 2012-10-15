class STest2 < CFunc::Struct
    define STest, :s,
      CFunc::Double, :xx
end

stest2 = STest2.new

stest2[:s][:x] = 10
stest2[:s][:y] = 20
stest2[:s][:z] = 4
stest2[:xx] = 0.0

assert_equal 10, stest2[:s][:x]
assert_equal 20, stest2[:s][:y]
assert_equal 4, stest2[:s][:z]
assert_equal 0.0, stest2[:xx]

result = CFunc::call(STest2, "cfunc_test_func2", stest2)

assert_equal 10, result[:s][:x]
assert_equal 20, result[:s][:y]
assert_equal 4, result[:s][:z]
assert_equal 7.5, result[:xx]


# BEGIN C
struct STest {
    int8_t x;
    int16_t y;
    int32_t z;
};

struct STest2 {
    struct STest s;
    double xx;
};

struct STest2 cfunc_test_func2(struct STest2 val) {
    val.xx = (double)(val.s.x + val.s.y) / val.s.z;
    return val;
};
