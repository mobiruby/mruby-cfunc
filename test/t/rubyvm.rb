
vm = CFunc::RubyVM.thread('_rubyvm1')

def sleep(sec)
  CFunc::call CFunc::Void, "sleep", CFunc::Int(sec)
end

task1 = vm.dispatch :sleep, 1
assert_not_equal CFunc::RubyVM::Task::FINISHED, task1.status
sleep 2
assert_equal CFunc::RubyVM::Task::FINISHED, task1.status
assert_equal 1, task1.result


task2 = vm.dispatch :sleep, 1
assert_not_equal CFunc::RubyVM::Task::FINISHED, task2.status
task2.wait
assert_equal CFunc::RubyVM::Task::FINISHED, task2.status
assert_equal 1, task2.result


task3 = vm.dispatch :reset_action1
assert_equal nil, task3.wait


[123, 123.45, 'ABC', :def, nil, true, false].each do |v|
  task4 = vm.dispatch :set_action1, v
  assert_equal v, task4.wait
  task5 = vm.dispatch :get_action1
  assert_equal v, task5.wait
end


assert_equal 1+2+3+4, vm.dispatch(:sum, [1, 2, 3, 4]).wait
