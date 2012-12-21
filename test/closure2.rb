mobiruby_test "CFunc::closure2" do
  compar = CFunc::Closure.new(CFunc::Int, [CFunc::Pointer(CFunc::Int), CFunc::Pointer(CFunc::Int)]) do |a, b|
    a.refer.to_i - b.refer.to_i
  end

  array = CFunc::CArray(CFunc::Int).new(10)
  assert_equal 10, array.size

  [5, 8, -1, -99, 99, 3, 1, 0, 10, 23].each_with_index do |v, i|
    array[i].value = v
  end

  CFunc::call(CFunc::Void, "qsort", array, CFunc::Int(10), CFunc::Int(CFunc::Int.size), compar)

  [-99, -1, 0, 1, 3, 5, 8, 10, 23, 99].each_with_index do |v, i|
    assert_equal v, array[i].value
  end
end
