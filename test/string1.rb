mobiruby_test "CFunc::String1" do
  str = "STRING"
  ptr = CFunc::Pointer.malloc(7)

  CFunc::call(CFunc::Void, "strcpy", ptr, str)
  assert_equal "STRING", ptr.to_s
  
  
  ptr2 = CFunc::UInt8[4]
  CFunc::call(CFunc::Void, "strcpy", ptr2, "too long")
  assert_equal "too ", ptr2.to_s
  
end
