assert_equal 8, CFunc::Double.size
assert 0 < CFunc::Double.align

assert_equal 16.45, CFunc::Double.new(16.45).to_f

dbl = CFunc::Double.new

dbl.value = 1.2
assert_equal 1.2, dbl.value
assert_equal 1, dbl.to_i
assert_equal 1.2, dbl.to_f

dbl.value = 1.23
dbl_ptr = dbl.to_pointer
assert dbl_ptr.is_a?(CFunc::Pointer)

assert CFunc::Double.refer(dbl_ptr).is_a?(CFunc::Double)
assert_equal 1.23, CFunc::Double.refer(dbl_ptr).value

10.times do |idx|
  dbl.value = idx
  assert_equal idx, CFunc::Double.refer(dbl_ptr).value
end

dbl.value = 128.256
assert_equal 128.256, CFunc::Double.get(dbl_ptr)

dbl = CFunc::Double.new(CFunc::Double.new(16.45))
assert_equal 16.45, dbl.value
