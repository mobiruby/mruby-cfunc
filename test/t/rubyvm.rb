
vm = CFunc::RubyVM.thread('_rubyvm1')
#assert_equal 0, vm.dispatch_sync(:action1a)

# it's not test. 
vm.dispatch_async :action1b,1
vm.dispatch_async :action1b,2
vm.dispatch_async :action1b,3
vm.dispatch_async :action1b,4
vm.dispatch_async :action1b,5
CFunc::call CFunc::Void, "sleep", CFunc::Int(2)
#assert_equal 1, vm.dispatch_sync(:action1a)
