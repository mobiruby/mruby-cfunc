assert_equal 2, CFunc::SInt16.size
assert 0 < CFunc::SInt16.align

assert_equal 16, CFunc::SInt16.new(16).to_i

sint = CFunc::SInt16.new

sint.value = 1
assert_equal 1, sint.value
assert_equal 1, sint.to_i
assert_equal 1.0, sint.to_f

sint.value = -1
assert_equal -1, sint.value

sint.value = 1
sint_ptr = sint.to_pointer
assert sint_ptr.is_a?(CFunc::Pointer)

assert CFunc::SInt16.refer(sint_ptr).is_a?(CFunc::SInt16)
assert_equal 1, CFunc::SInt16.refer(sint_ptr).value

10.times do |idx|
  sint.value = idx
  assert_equal idx, CFunc::SInt16.refer(sint_ptr).value
end

sint.value = 32767
assert_equal 32767, CFunc::SInt16.get(sint_ptr)

CFunc::SInt16.set(sint_ptr, 32768)
assert_not_equal 32768, sint.value

CFunc::SInt16.set(sint_ptr, -32769)
assert_not_equal -32769, sint.value