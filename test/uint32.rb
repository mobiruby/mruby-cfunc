mobiruby_test "CFunc::UInt32" do
  assert_equal 4, CFunc::UInt32.size
  assert 0 < CFunc::UInt32.align

  assert_equal 16, CFunc::UInt32.new(16).to_i

  uint = CFunc::UInt32.new

  uint.value = 1
  assert_equal 1, uint.value
  assert_equal 1, uint.to_i
  assert_equal 1.0, uint.to_f

  uint.value = 1
  uint_ptr = uint.addr
  assert uint_ptr.is_a?(CFunc::Pointer)

  assert CFunc::UInt32.refer(uint_ptr).is_a?(CFunc::UInt32)
  assert_equal 1, CFunc::UInt32.refer(uint_ptr).value

  10.times do |idx|
    uint.value = idx
    assert_equal idx, CFunc::UInt32.refer(uint_ptr).value
  end

  uint32_max = 4294967295
  uint32_min = 0

  uint.value = uint32_max
  assert_equal uint32_max, uint.value
  assert_equal uint32_max, CFunc::UInt32.get(uint_ptr)

  CFunc::UInt32.set(uint_ptr, uint32_max+1)
  assert_not_equal uint32_max, uint.value

  CFunc::UInt32.set(uint_ptr, uint32_min-1)
  assert_not_equal (uint32_min-1), uint.value

  uint = CFunc::UInt32.new(CFunc::SInt16.new(16))
  assert_equal 16, uint.value
end
