mobiruby_test "CFunc::UInt16" do
  assert_equal 2, CFunc::UInt16.size
  assert 0 < CFunc::UInt16.align

  assert_equal 16, CFunc::UInt16.new(16).to_i

  uint = CFunc::UInt16.new

  uint.value = 1
  assert_equal 1, uint.value
  assert_equal 1, uint.to_i
  assert_equal 1.0, uint.to_f

  uint.value = 1
  uint_ptr = uint.addr
  assert uint_ptr.is_a?(CFunc::Pointer)

  assert CFunc::UInt16.refer(uint_ptr).is_a?(CFunc::UInt16)
  assert_equal 1, CFunc::UInt16.refer(uint_ptr).value

  10.times do |idx|
    uint.value = idx
    assert_equal idx, CFunc::UInt16.refer(uint_ptr).value
  end

  uint.value = 65535
  assert_equal 65535, CFunc::UInt16.get(uint_ptr)

  CFunc::UInt16.set(uint_ptr, 65536)
  assert_not_equal 65536, uint.value

  CFunc::UInt16.set(uint_ptr, -1)
  assert_not_equal (-1), uint.value

  uint = CFunc::UInt16.new(CFunc::SInt16.new(16))
  assert_equal 16, uint.value
end
