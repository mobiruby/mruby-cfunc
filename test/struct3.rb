unless Object.const_defined?(:STest4)
  CFunc::Struct.define :STest4,
    CFunc::SInt8[10], :x,
    CFunc::SInt16, :y,
    CFunc::SInt32, :z
end

mobiruby_test "CFunc::Struct3" do
  assert_equal 16, STest4.size
  assert 0 < STest4.align
  stest = STest4.new

  10.times do |i|
    stest[:x][i] = 10 + i
  end
  stest[:y] = 20
  stest[:z] = 0

  10.times do |i|
    assert_equal 10 + i, stest[:x][i].value
  end
  assert_equal 20, stest[:y]
  assert_equal 0, stest[:z]
end
