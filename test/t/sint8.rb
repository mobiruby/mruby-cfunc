assert_equal 1, CFunc::SInt8.size
assert 0 < CFunc::SInt8.align

assert_equal 16, CFunc::SInt8.new(16).to_i

sint = CFunc::SInt8.new

sint.value = 1
assert_equal 1, sint.value
assert_equal 1, sint.to_i
assert_equal 1.0, sint.to_f

sint.value = -1
assert_equal -1, sint.value

sint.value = 1
sint_ptr = sint.addr
assert sint_ptr.is_a?(CFunc::Pointer)

assert CFunc::SInt8.refer(sint_ptr).is_a?(CFunc::SInt8)
assert_equal 1, CFunc::SInt8.refer(sint_ptr).value

10.times do |idx|
  sint.value = idx
  assert_equal idx, CFunc::SInt8.refer(sint_ptr).value
end

sint.value = 127
assert_equal 127, CFunc::SInt8.get(sint_ptr)

CFunc::SInt8.set(sint_ptr, 128)
assert_not_equal 128, sint.value

CFunc::SInt8.set(sint_ptr, -129)
assert_not_equal -129, sint.value

sint = CFunc::SInt8.new(CFunc::SInt16.new(16))
assert_equal 16, sint.value
