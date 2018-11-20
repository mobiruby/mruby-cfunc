#include "mruby.h"
#include "mruby/dump.h"
#include "mruby/proc.h"
#include "mruby/compile.h"

extern void test_func_ref();
extern uint8_t const mruby_data__rubyvm1[];

void
mrb_mruby_cfunc_gem_test(mrb_state *mrb)
{
    uint8_t const *ptr;
    test_func_ref();
    ptr = mruby_data__rubyvm1;
    (void)ptr;
}
