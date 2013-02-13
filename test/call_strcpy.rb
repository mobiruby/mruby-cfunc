mobiruby_test "CFunc::call_strcpy" do
  ptr = CFunc::Pointer.malloc(12)

  if CFunc::Platform.is_win32?
    result = CFunc::libcall(CFunc::Pointer, "msvcrt.dll", "strcpy", ptr, "Hello")
  else
    result = CFunc::call(CFunc::Pointer, "strcpy", ptr, "Hello")
  end
  assert_equal "Hello", result.to_s
  assert_equal "Hello", ptr.to_s

  if CFunc::Platform.is_win32?
    result = CFunc::libcall(CFunc::Pointer, "msvcrt.dll", :strcat, ptr, " World")
  else
    result = CFunc::call(CFunc::Pointer, :strcat, ptr, " World")
  end
  assert_equal "Hello World", result.to_s
  assert_equal "Hello World", ptr.to_s
end
