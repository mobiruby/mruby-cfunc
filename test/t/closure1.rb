closure = CFunc::Closure.new(CFunc::Int, [CFunc::Int, CFunc::Int]) do |a, b|
  a.value * b.value
end

result = CFunc::call(CFunc::Int, "cfunc_test_func3", closure)
assert_equal 200, result.value

############
# BEGIN C
int cfunc_test_func3(int (*func)(int, int)) {
    return func(10, 20);
}