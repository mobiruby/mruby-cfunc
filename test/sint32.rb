mobiruby_test "CFunc::SInt32" do
  assert_equal 4, CFunc::SInt32.size
  assert 0 < CFunc::SInt32.align

  sint = CFunc::SInt32.new

  sint.value = 1
  assert_equal 1, sint.value
  assert_equal 1, sint.to_i
  assert_equal 1.0, sint.to_f

  sint.value = -1
  assert_equal (-1), sint.value

  sint.value = 1
  sint_ptr = sint.addr
  assert sint_ptr.is_a?(CFunc::Pointer)

  assert CFunc::SInt32.refer(sint_ptr).is_a?(CFunc::SInt32)
  assert_equal 1, CFunc::SInt32.refer(sint_ptr).value

  10.times do |idx|
    sint.value = idx
    assert_equal idx, CFunc::SInt32.refer(sint_ptr).value
  end

  sint32_max = 2147483647
  sint32_min = -2147483647

  sint.value = sint32_max
  assert_equal sint32_max, sint.value
  assert_equal sint32_max, CFunc::SInt32.get(sint_ptr)

  CFunc::SInt32.set(sint_ptr, sint32_max+1)
  assert_not_equal sint32_max+1, sint.value

  sint.value = sint32_min
  assert_equal sint32_min, sint.value
  assert_equal sint32_min, CFunc::SInt32.get(sint_ptr)

  CFunc::SInt32.set(sint_ptr, sint32_min-1)
  assert_not_equal sint32_min , sint.value

  sint = CFunc::SInt32.new(CFunc::SInt16.new(16))
  assert_equal 16, sint.value
end
