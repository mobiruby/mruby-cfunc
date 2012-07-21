class IntValueTest1 < TestCase
  def test_10_parameters
    eq 4, CFunc::Int.size
    eq 4, CFunc::Int.align
  end

  def test_20_set_and_verify
    i = CFunc::Int.new
    i.value = 1
    eq 1, i.value
  end

  def test_30_array
    ci = CFunc::CArray(CFunc::Int).new(10)
    eq 10, ci.size
    for i in 0..9
      ci[i].value = i ** 2
    end
    for i in 0..9
      eq i ** 2, ci[i].value
    end
  end
end
IntValueTest1.run


class StringValueTest1 < TestCase
  def test_10_parameters
    @str = "STRING"
    @ptr = CFunc::Pointer.malloc(7)
    CFunc::call(CFunc::Void, "strcpy", @ptr, @str)
    eq "STRING", @ptr.to_s
  end

end
StringValueTest1.run


class STest < CFunc::Struct
  define CFunc::SInt8, :x,
    CFunc::SInt16, :y,
    CFunc::SInt32, :z
end

class StructTest1 < TestCase
  def test_10_create_instance
    @stest = ::STest.new
  end

  def test_20_set_values
    @stest[:x] = 10
    @stest[:y] = 20
    @stest[:z] = 0
  end

  def test_30_verify_values
    eq 10, @stest[:x]
    eq 20, @stest[:y]
    eq 0, @stest[:z]
  end

  def test_40_change_values
    @stest[:x] = 25
    @stest[:y] = 50
    @stest[:z] = 100
  end

  def test_50_verify_values
    eq 25, @stest[:x]
    eq 50, @stest[:y]
    eq 100, @stest[:z]
  end
end
StructTest1.run


class StructAndCallTest1 < TestCase
  def test_10_create_instance
    @stest = ::STest.new
  end

  def test_20_set_values
    @stest[:x] = 10
    @stest[:y] = 20
    @stest[:z] = 0
  end

  def test_30_call_test_func1
    @result = CFunc::call(STest, "cfunc_test_func1", @stest)
  end

  def test_40_verify_result
    a @result.is_a?(STest)
    eq 10, @result[:x]
    eq 20, @result[:y]
    eq 30, @result[:z]
  end
end
StructAndCallTest1.run


class STest2 < CFunc::Struct
    define STest, :s,
      CFunc::Double, :xx
end


class StructTest2 < TestCase
  def test_10_create_instance
    @stest2 = STest2.new
  end

  def test_20_set_values
    @stest2[:s][:x] = 10
    @stest2[:s][:y] = 20
    @stest2[:s][:z] = 4
    @stest2[:xx] = 0.0
  end

  def test_30_verify_values
    eq 10, @stest2[:s][:x]
    eq 20, @stest2[:s][:y]
    eq 4, @stest2[:s][:z]
    eq 0.0, @stest2[:xx]
  end

  def test_40_call_test_func2
    @result = CFunc::call(STest2, "cfunc_test_func2", @stest2)
  end

  def test_50_verift_result
    eq 10, @result[:s][:x]
    eq 20, @result[:s][:y]
    eq 4, @result[:s][:z]
    eq 7.5, @result[:xx]
  end
end
StructTest2.run


class ClosureTest1 < TestCase
  def test_10_define_closure
    @closure = CFunc::Closure.new(CFunc::Int, [CFunc::Int, CFunc::Int]) do |a, b|
      a.value * b.value
    end
  end

  def test_20_call_closure_from_c
    @result = CFunc::call(CFunc::Int, "cfunc_test_func3", @closure)
  end

  def test_30_verify
    eq 200, @result.value
  end
end
ClosureTest1.run
