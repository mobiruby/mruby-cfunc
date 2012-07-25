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

void
init_cfunc_rb(mrb_state *mrb);


void init_cfunc_module(mrb_state *mrb)
{
    if(mrb->ud == NULL) {
        mrb->ud = malloc(sizeof(CFUNC_MRB_STATE_UD));
    }
    
    struct RClass *module = mrb_define_module(mrb, "CFunc");

    init_cfunc_type(mrb, module);
    init_cfunc_pointer(mrb, module);
    init_cfunc_struct(mrb, module);
    init_cfunc_closure(mrb, module);
    init_cfunc_call(mrb, module);

    init_cfunc_rb(mrb);
}
