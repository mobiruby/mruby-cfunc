
vm = CFunc::RubyVM.thread('_rubyvm1')
#assert_equal 0, vm.dispatch_sync(:action1a)

# it's not test. 
vm.dispatch_async :action1b, 1
vm.dispatch_async :action1b, 2
vm.dispatch_async :action1b, 3
vm.dispatch_async :action1b, 4
vm.dispatch_async :sum, [1, 2, 3]
vm.dispatch_async :puts_str, "ABC"
CFunc::call CFunc::Void, "sleep", CFunc::Int(1)
#assert_equal 1, vm.dispatch_sync(:action1a)
