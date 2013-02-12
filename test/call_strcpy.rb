mobiruby_test "CFunc::call_strcpy" do
  skip if CFunc::Platform.is_win32?

  ptr = CFunc::Pointer.malloc(12)

  result = CFunc::call(CFunc::Pointer, "strcpy", ptr, "Hello")
  assert_equal "Hello", result.to_s
  assert_equal "Hello", ptr.to_s

  result = CFunc::call(CFunc::Pointer, :strcat, ptr, " World")
  assert_equal "Hello World", result.to_s
  assert_equal "Hello World", ptr.to_s
end
