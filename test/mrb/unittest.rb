
class TestCase
  attr_reader :errors

  def initialize
    @errors = []
    @current_test = nil
  end

  def assert_equal(exp, act, msg = nil)
    if exp == act
      print '.'
    else
      print '*'
      @errors << [@current_test, msg]
    end
  end

  def assert(test, msg = nil)
    if test
      print '.'
    else
      print '*'
      @errors << [@current_test, msg]
    end
  end

  def a(test)
    assert(test)
  end

  def eq(exp, act, msg = nil)
    assert_equal(exp, act, msg)
  end

  def run
    @errors = []
    puts self.class
    self.methods.select{|m| m.to_s[0, 5] == 'test_' }.sort.each do |m|
      @current_test = m
      begin
        print "  #{m}: "
        send(m)
        puts ''
      rescue => e
        puts 'E'
        @errors << [@current_test, e]
      end
      @current_test = nil
    end
    puts
    unless @errors.empty?
      @errors.each do |line|
        p line
      end
    end
  end

  def self.run
    self.new.run
  end
end

