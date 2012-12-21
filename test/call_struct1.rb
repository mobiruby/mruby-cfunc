unless Object.const_defined?(:STest)
  class STest < CFunc::Struct
    define CFunc::SInt8, :x,
      CFunc::SInt16, :y,
      CFunc::SInt32, :z
  end
end

mobiruby_test "CFunc::call_struct1" do
  stest = ::STest.new

  assert_equal 10, stest[:x] = 10
  assert_equal 20, stest[:y] = 20
  assert_equal 0, stest[:z] = 0

  result = CFunc::call(STest, "cfunc_test_func1", stest)

  assert result.is_a?(STest)
  assert_equal 10, result[:x]
  assert_equal 20, result[:y]
  assert_equal 30, result[:z]
end
