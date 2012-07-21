#include "cfunc.h"

#include "mruby.h"
#include "mruby/dump.h"
#include "mruby/proc.h"
#include "mruby/compile.h"


void
init_unittest(mrb_state *mrb);

void
init_cfunc_test(mrb_state *mrb);


int main(int argc, char *argv[])
{
    mrb_state *mrb = mrb_open();

    init_cfunc_module(mrb);

    init_unittest(mrb);
    if (mrb->exc) {
        mrb_p(mrb, mrb_obj_value(mrb->exc));
    }

    init_cfunc_test(mrb);
    if (mrb->exc) {
        mrb_p(mrb, mrb_obj_value(mrb->exc));
    }
}


struct STest {
    int8_t x;
    int16_t y;
    int32_t z;
};


struct STest2 {
    struct STest s;
    double xx;
};


struct STest cfunc_test_func1(struct STest val) {
    val.z = val.x + val.y;
    return val;
};


struct STest2 cfunc_test_func2(struct STest2 val) {
    val.xx = (double)(val.s.x + val.s.y) / val.s.z;
    return val;
};


int cfunc_test_func3(int (*func)(int, int)) {
    return func(10, 20);
}
