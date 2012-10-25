//
//  CFunc initializer method in C
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc.h"

#include "cfunc_call.h"
#include "cfunc_pointer.h"
#include "cfunc_struct.h"
#include "cfunc_closure.h"
#include "cfunc_type.h"
#include "cfunc_rubyvm.h"

#include "mruby/proc.h"
#include "mruby/dump.h"

#include <setjmp.h>

#ifdef USE_MRBC_DATA
extern const char mruby_data_cfunc_rb[];
#endif

size_t cfunc_state_offset = 0;

// generate from mrb/cfunc_rb.rb
void
init_cfunc_rb(mrb_state *mrb);

mrb_value
cfunc_mrb_state(mrb_state *mrb, mrb_value klass)
{
    return cfunc_pointer_new_with_pointer(mrb, mrb, false);
}


void init_cfunc_module(mrb_state *mrb)
{
    struct RClass *ns = mrb_define_module(mrb, "CFunc");
    cfunc_state(mrb)->namespace = ns;

    init_cfunc_type(mrb, ns);
    init_cfunc_pointer(mrb, ns);
    init_cfunc_struct(mrb, ns);
    init_cfunc_closure(mrb, ns);
    init_cfunc_call(mrb, ns);
    init_cfunc_rubyvm(mrb, ns);

    mrb_define_class_method(mrb, ns, "mrb_state", cfunc_mrb_state, ARGS_NONE());
    
#ifdef USE_MRBC_DATA
    int n = mrb_read_irep(mrb, mruby_data_cfunc_rb);
    if (n >= 0) {
        mrb_irep *irep = mrb->irep[n];
        struct RProc *proc = mrb_proc_new(mrb, irep);
        proc->target_class = mrb->object_class;
        mrb_run(mrb, proc, mrb_top_self(mrb));
    }
    else if (mrb->exc) {
        longjmp(*(jmp_buf*)mrb->jmp, 1);
    }
#else
    init_cfunc_rb(mrb);
#endif

}
