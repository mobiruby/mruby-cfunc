$action1_result = 0

def action1a
  puts "action1a"
  $action1_result
end

def action1b(a)
  puts "action1b: #{$action1_result += 1}, #{a*2}"
end

def puts_str(str)
  puts str
end

def sum(ary)
  result = ary.inject(0) { | sum, i | sum + i }
  puts "[#{ary.join(',')}].sum = #{result}"
end


puts "THREAD"