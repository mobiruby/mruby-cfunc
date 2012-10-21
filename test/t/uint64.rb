def assert_raise(*exps, &block)
  flg = false # escape https://github.com/mruby/mruby/issues/497
  begin
    yield
  rescue => e
    flg = true if exps.include?(e.class)
  end
  assert(flg, "Didn't raise #{exps}")
end


assert_equal 8, CFunc::UInt64.size
assert_equal 8, CFunc::UInt64.align

uint = CFunc::UInt64.new


uint.value = 1
assert_equal 1, uint.low
assert_equal 0, uint.high
assert_equal 0, uint.high
assert_equal 1, uint.value
assert_equal 1, uint.to_i
assert_equal 1.0, uint.to_f

uint.low = 2
uint.high = 1
assert_equal 2, uint.low
assert_equal 1, uint.high
assert_equal (1<<32)+2, uint.value
assert_equal "4294967298", uint.to_s
uint_ptr = uint.to_pointer
assert uint_ptr.is_a?(CFunc::Pointer)
assert CFunc::UInt64.refer(uint_ptr).is_a?(CFunc::UInt64)
