class STest < CFunc::Struct
  define CFunc::SInt8, :x,
    CFunc::SInt16, :y,
    CFunc::SInt32, :z
end

assert_equal 8, STest.size
assert 0 < STest.align
stest = STest.new

stest[:x] = 10
stest[:y] = 20
stest[:z] = 0

assert_equal 10, stest[:x]
assert_equal 20, stest[:y]
assert_equal 0, stest[:z]

stest[:x] = 25
stest[:y] = 50
stest[:z] = 100

assert_equal 25, stest[:x]
assert_equal 50, stest[:y]
assert_equal 100, stest[:z]
