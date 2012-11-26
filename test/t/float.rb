assert_equal 4, CFunc::Float.size
assert 0 < CFunc::Float.align

assert_equal 0.125, CFunc::Float.new(0.125).to_f

flt = CFunc::Float.new

flt.value = 0.5
assert_equal 0.5, flt.value
assert_equal 0, flt.to_i
assert_equal 0.5, flt.to_f

flt.value = 1.25
flt_ptr = flt.to_pointer
assert flt_ptr.is_a?(CFunc::Pointer)

assert CFunc::Float.refer(flt_ptr).is_a?(CFunc::Float)
assert_equal 1.25, CFunc::Float.refer(flt_ptr).value

10.times do |idx|
  flt.value = idx+0.125
  assert_equal idx+0.125, CFunc::Float.refer(flt_ptr).value
end

flt.value = 0.0625
assert_equal 0.0625, CFunc::Float.get(flt_ptr)
