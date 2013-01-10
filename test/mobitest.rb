$ok_test, $ko_test, $kill_test = 0, 0, 0
$asserts = []

class MobiRubyTest
  def initialize(label)
    @label = label
    @index = 0
  end

  def run(&block)
    begin
      instance_eval(&block)
    rescue Exception => e
      $asserts.push "Error: #{@label} ##{@index} #{e}"
      $kill_test += 1
      print('X')
    end
  end

  def assert(result, label='')
    @index += 1
    if !result
      $asserts.push "Fail: #{@label} ##{@index} #{label}"
      $ko_test += 1
      print('F')
    else
      $ok_test += 1
      print('.')
    end
  end

  def assert_equal(a, b)
    assert(a===b, "<#{a.inspect}> expected but was <#{b.inspect}>")
  end

  def assert_not_equal(a, b)
    assert(!(a===b), "<#{a.inspect}> not expected but was <#{b.inspect}>")
  end
end

def mobiruby_test(label, &block)
  MobiRubyTest.new(label).run(&block)
end
