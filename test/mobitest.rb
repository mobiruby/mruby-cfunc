$ok_test, $ko_test, $kill_test = 0, 0, 0
$asserts = []

class MobiRubyTestSkip < NotImplementedError
end

class MobiRubyTest
  def initialize(label)
    @label = label
    @index = 0
  end

  def run(&block)
    begin
      instance_eval(&block)
    rescue Exception => e
      if e.class.to_s != 'MobiRubyTestSkip'
        $asserts.push "Error: #{@label} ##{@index} #{e}"
        $kill_test += 1
        print('X')
      else
        $asserts.push "Skip: #{@label} ##{@index}"
        print('?')
      end
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

  def skip
    raise MobiRubyTestSkip.new
  end

  def assert_equal(a, b)
    assert(a===b, "<#{a.inspect}> expected but was <#{b.inspect}>")
  end

  def assert_not_equal(a, b)
    assert(!(a===b), "<#{a.inspect}> not expected but was <#{b.inspect}>")
  end

  def assert_raise(*args)
    failure_message = args.last.is_a?(String) ? args.pop : nil
    begin
      yield
      assert(false, "#{args} expected but none was thrown.")
    rescue Exception => e
      assert(args.include?(e.class), "#{args} expected but was #{e}.")
    end
  end
end

def mobiruby_test(label, &block)
  MobiRubyTest.new(label).run(&block)
end
