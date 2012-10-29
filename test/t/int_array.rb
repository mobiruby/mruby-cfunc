ci = CFunc::CArray(CFunc::Int).new(10)
assert_equal 10, ci.size

for i in 0..9
  ci[i].value = i ** 2
end

for i in 0..9
  assert_equal i ** 2, ci[i].value
end

