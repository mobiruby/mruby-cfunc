val = 5
callback = CFunc::Closure.new(CFunc::Int, [CFunc::Int, CFunc::Int]) do |a, b|
  a.to_i * b.to_i * val
end

result = CFunc::call(CFunc::Int, "cfunc_test_func3", callback)
assert_equal 10*20*5, result.to_i


############
# BEGIN C
int cfunc_test_func3(int (*func)(int, int)) {
    return func(10, 20);
}