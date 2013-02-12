unless Object.const_defined?(:STest)
  class STest < CFunc::Struct
    define CFunc::SInt8, :x,
      CFunc::SInt16, :y,
      CFunc::SInt32, :z
  end
end

unless Object.const_defined?(:STest2)
  class ::STest2 < CFunc::Struct
    define ::STest, :s,
      CFunc::Double, :xx
  end
end

mobiruby_test "CFunc::call_struct2" do
  skip if CFunc::Platform.is_win32?

  stest2 = ::STest2.new

  assert_equal 10, stest2[:s][:x] = 10
  assert_equal 20, stest2[:s][:y] = 20
  assert_equal 4, stest2[:s][:z] = 4
  assert_equal 0.0, stest2[:xx] = 0.0

  assert_equal 10, stest2[:s][:x]
  assert_equal 20, stest2[:s][:y]
  assert_equal 4, stest2[:s][:z]
  assert_equal 0.0, stest2[:xx]

  result = CFunc::call(STest2, "cfunc_test_func2", stest2)

  assert result.is_a?(STest2)
  assert_equal 7.5, result[:xx]

  assert result[:s].is_a?(STest)
  assert_equal 10, result[:s][:x]
  assert_equal 20, result[:s][:y]
  assert_equal 4, result[:s][:z]
end
