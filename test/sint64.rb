mobiruby_test "CFunc::SInt64" do
  assert_equal 8, CFunc::SInt64.size
  assert 0 < CFunc::SInt64.align

  assert_equal 16, CFunc::SInt64.new(16).to_i

  sint = CFunc::SInt64.new

  sint.value = 1
  assert_equal 1, sint.value
  assert_equal 1, sint.to_i
  assert_equal 1.0, sint.to_f

  sint.value = -1
  assert_equal (-1), sint.value

  sint.value = 1
  sint_ptr = sint.addr
  assert sint_ptr.is_a?(CFunc::Pointer)

  assert CFunc::SInt64.refer(sint_ptr).is_a?(CFunc::SInt64)
  assert_equal 1, CFunc::SInt64.refer(sint_ptr).value

  10.times do |idx|
    sint.value = idx
    assert_equal idx, CFunc::SInt64.refer(sint_ptr).value
  end

  sint.value = 0x7fffffff
  assert_equal 0x7fffffff, sint.low
  assert_equal 0x00000000, sint.high
  assert_equal 0x7fffffff, CFunc::SInt64.get(sint_ptr)

  sint.value = -1 
  assert_equal 0xffffffff, sint.low
  assert_equal 0xffffffff, sint.high
  assert_equal -1, CFunc::SInt64.get(sint_ptr)

  sint = CFunc::SInt64.new(CFunc::SInt16.new(16))
  assert_equal 16, sint.value
end
