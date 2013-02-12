mobiruby_test "CFunc::define_cfunc" do
  skip if CFunc::Platform.is_win32?

  CFunc::define_function CFunc::Pointer, "strcpy", CFunc::Pointer, CFunc::Pointer
  CFunc::define_function CFunc::Pointer, "strcat"
  CFunc::define_function CFunc::Int, "cfunc_test_func4", CFunc::Int, CFunc::Int

  ptr = CFunc::Pointer.malloc(12)

  result = CFunc[:strcpy].call(ptr, "Hello")
  assert_equal "Hello", result.to_s
  assert_equal "Hello", ptr.to_s

  result = CFunc["strcat"].call(ptr, " World")
  assert_equal "Hello World", result.to_s
  assert_equal "Hello World", ptr.to_s


  result = CFunc["cfunc_test_func4"].call(2, 3)
  assert_equal 6, result.to_i
end
