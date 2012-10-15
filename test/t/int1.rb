assert_equal 4, CFunc::Int.size
assert_equal 4, CFunc::Int.align

i = CFunc::Int.new
i.value = 1
assert_equal 1, i.value
assert_equal 1, i.to_i
