assert_equal 1, CFunc::UInt8.size
assert 0 < CFunc::UInt8.align

uint = CFunc::UInt8.new

uint.value = 1
assert_equal 1, uint.value
assert_equal 1, uint.to_i
assert_equal 1.0, uint.to_f

uint.value = 1
UInt_ptr = uint.to_pointer
assert UInt_ptr.is_a?(CFunc::Pointer)

assert CFunc::UInt8.refer(UInt_ptr).is_a?(CFunc::UInt8)
assert_equal 1, CFunc::UInt8.refer(UInt_ptr).value

10.times do |idx|
  uint.value = idx
  assert_equal idx, CFunc::UInt8.refer(UInt_ptr).value
end

uint.value = 127
assert_equal 127, CFunc::UInt8.get(UInt_ptr)

CFunc::UInt8.set(UInt_ptr, 256)
assert_not_equal 256, uint.value

CFunc::UInt8.set(UInt_ptr, -1)
assert_not_equal -1, uint.value

