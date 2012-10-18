assert_equal 4, CFunc::SInt32.size
assert_equal 4, CFunc::SInt32.align

sint = CFunc::SInt32.new

sint.value = 1
assert_equal 1, sint.value
assert_equal 1, sint.to_i
assert_equal 1.0, sint.to_f

sint.value = -1
assert_equal -1, sint.value

sint.value = 1
sint_ptr = sint.to_pointer
assert sint_ptr.is_a?(CFunc::Pointer)

assert CFunc::SInt32.refer(sint_ptr).is_a?(CFunc::SInt32)
assert_equal 1, CFunc::SInt32.refer(sint_ptr).value

10.times do |idx|
  sint.value = idx
  assert_equal idx, CFunc::SInt32.refer(sint_ptr).value
end

sint.value = (1<<31)-1
assert_equal (1<<31)-1, sint.value
assert_equal (1<<31)-1, CFunc::SInt32.get(sint_ptr)

CFunc::SInt32.set(sint_ptr, (1<<31))
assert_not_equal (1<<31), sint.value

sint.value = (-1<<31)
assert_equal (-1<<31), sint.value
assert_equal (-1<<31), CFunc::SInt32.get(sint_ptr)

CFunc::SInt32.set(sint_ptr, (-1<<31)-1)
assert_not_equal (-1<<31)-1, sint.value
