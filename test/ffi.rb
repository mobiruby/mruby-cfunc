mobiruby_test "FFI::MemoryPointer(:int)" do
  num = FFI::MemoryPointer.new(:int)

  num.write_int(0)
  assert_equal num.read_int(), 0
  
  num.write_int(42)
  assert_equal num.read_int(), 42

end

mobiruby_test "FFI::MemoryPointer(:string)" do
  num = FFI::MemoryPointer.new(5)

  num.write_string("A string")
  assert_equal(num.read_string(), "A string")
end

mobiruby_test "FFI::MemoryPointer(array of int)" do
  arr = FFI::MemoryPointer.new(:int, 3)
  arr.write_array_of_int([1, 2, 3])
  
  assert_equal(arr.read_array_of_int(), [1,2,5])
end


