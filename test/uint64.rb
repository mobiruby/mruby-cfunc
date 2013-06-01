mobiruby_test "CFunc::UInt64" do
  assert_equal 8, CFunc::UInt64.size
  assert 0 < CFunc::UInt64.align

  assert_equal 16, CFunc::UInt64.new(16).to_i

  uint = CFunc::UInt64.new

  uint.value = 1
  assert_equal 1, uint.low
  assert_equal 0, uint.high
  assert_equal 1, uint.value
  assert_equal 1, uint.to_i
  assert_equal 1.0, uint.to_f

  uint.low = 2
  uint.high = 1
  assert_equal 2, uint.low
  assert_equal 1, uint.high
  assert_equal "4294967298", uint.to_s
  uint_ptr = uint.addr
  assert uint_ptr.is_a?(CFunc::Pointer)
  assert CFunc::UInt64.refer(uint_ptr).is_a?(CFunc::UInt64)

  uint = CFunc::UInt64.new(CFunc::SInt16.new(16))
  assert_equal 16, uint.value
  
  
  # divide test
  uint.low = 0xFFFFFFFF
  uint.high = 0xFF
  
  err = nil
  begin
    uint.value
  rescue => ex
    err = ex
  end
  
  assert_equal 'TypeError', err.class.to_s
  assert_equal "too big. Use low, high", err.message
  
  assert_equal uint.divide(1000), 1099511627
end
