mobiruby_test "CFunc::Function pointer" do
  dlh = CFunc::call(CFunc::Pointer, "dlopen", nil, nil)

  strcpy_ptr = CFunc::call(CFunc::Pointer, "dlsym", dlh, "strcpy")
  strcat_ptr = CFunc::call(CFunc::Pointer, :dlsym, dlh, "strcat")

  ptr = CFunc::Pointer.malloc(12)

  result = CFunc::call(CFunc::Pointer, strcpy_ptr, ptr, "Hello")
  assert_equal "Hello", result.to_s
  assert_equal "Hello", ptr.to_s

  result = CFunc::call(CFunc::Pointer, "strcat", ptr, " World")
  assert_equal "Hello World", result.to_s
  assert_equal "Hello World", ptr.to_s
end
