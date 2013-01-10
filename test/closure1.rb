mobiruby_test "CFunc::closure1" do
  val = 5
  callback = CFunc::Closure.new(CFunc::Int, [CFunc::SInt32, CFunc::SInt32]) do |a, b|
    a.to_i * b.to_i * val
  end

  result = CFunc::call(CFunc::Int, "cfunc_test_func3", callback)
  assert_equal 10*20*5, result.to_i
end
