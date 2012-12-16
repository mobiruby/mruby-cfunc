assert_equal 4, CFunc::Int.size
assert 0 < CFunc::Int.align

int = CFunc::Int.new

int.value = 1
assert_equal 1, int.value
assert_equal 1, int.to_i

assert_equal 1.0, int.to_f

int_ptr = int.addr
assert int_ptr.is_a?(CFunc::Pointer)

assert CFunc::Int.refer(int_ptr).is_a?(CFunc::Int)
assert_equal 1, CFunc::Int.refer(int_ptr).value

10.times do |idx|
  int.value = idx
  assert_equal idx, CFunc::Int.refer(int_ptr).value
end

int.value = 123
assert_equal 123, CFunc::Int.get(int_ptr)

CFunc::Int.set(int_ptr, 456)
assert_equal 456, int.value

int = CFunc::Int.new(CFunc::SInt16.new(16))
assert_equal 16, int.value
