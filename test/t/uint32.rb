assert_equal 4, CFunc::UInt32.size
assert_equal 4, CFunc::UInt32.align

uint = CFunc::UInt32.new

uint.value = 1
assert_equal 1, uint.value
assert_equal 1, uint.to_i
assert_equal 1.0, uint.to_f

uint.value = 1
uint_ptr = uint.to_pointer
assert uint_ptr.is_a?(CFunc::Pointer)

assert CFunc::UInt32.refer(uint_ptr).is_a?(CFunc::UInt32)
assert_equal 1, CFunc::UInt32.refer(uint_ptr).value

10.times do |idx|
  uint.value = idx
  assert_equal idx, CFunc::UInt32.refer(uint_ptr).value
end

uint.value = 2**32-1
assert_equal 2**32-1, uint.value
assert_equal 2**32-1, CFunc::UInt32.get(uint_ptr)

CFunc::UInt32.set(uint_ptr, 2**32)
assert_not_equal 2**32, uint.value

CFunc::UInt32.set(uint_ptr, -1)
assert_not_equal -1, uint.value