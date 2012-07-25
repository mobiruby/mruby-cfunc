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

size_t cfunc_state_offset = 0;

// generate from mrb/cfunc_rb.rb
void
init_cfunc_rb(mrb_state *mrb);


void init_cfunc_module(mrb_state *mrb)
{
    struct RClass *ns = mrb_define_module(mrb, "CFunc");
    cfunc_state(mrb)->namespace = ns;

    init_cfunc_type(mrb, ns);
    init_cfunc_pointer(mrb, ns);
    init_cfunc_struct(mrb, ns);
    init_cfunc_closure(mrb, ns);
    init_cfunc_call(mrb, ns);

    init_cfunc_rb(mrb);
}
