assert_equal 8, CFunc::SInt64.size
assert_equal 8, CFunc::SInt64.align

sint = CFunc::SInt64.new

sint.value = 1
assert_equal 1, sint.value
assert_equal 1, sint.to_i
assert_equal 1.0, sint.to_f

sint.value = -1
assert_equal -1, sint.value

sint.value = 1
sint_ptr = sint.to_pointer
assert sint_ptr.is_a?(CFunc::Pointer)

assert CFunc::SInt64.refer(sint_ptr).is_a?(CFunc::SInt64)
assert_equal 1, CFunc::SInt64.refer(sint_ptr).value

10.times do |idx|
  sint.value = idx
  assert_equal idx, CFunc::SInt64.refer(sint_ptr).value
end

sint.value = 9223372036854775807 # MAX
assert_equal 9223372036854775807, sint.value
assert_equal 9223372036854775807, CFunc::SInt64.get(sint_ptr)

sint.value = -9223372036854775808 # MIN
assert_equal -9223372036854775808, sint.value
assert_equal -9223372036854775808, CFunc::SInt64.get(sint_ptr)
