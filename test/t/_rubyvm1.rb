$action1_result = 0

def action1a
  puts "action1a"
  $action1_result
end

def action1b(a)
  puts "action1b: #{$action1_result += 1}, #{a*2}"
end

puts "THREAD"