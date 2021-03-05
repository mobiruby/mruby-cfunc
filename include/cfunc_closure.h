//
//  CFunc::Closure class
// 
//  See Copyright Notice in cfunc.h
//

#ifndef cfunc_closure_h
#define cfunc_closure_h

#include "cfunc.h"
#include "cfunc_pointer.h"

#include "ffi.h"

#include <stdbool.h>


struct cfunc_closure_data {
    CFUNC_TYPE_HEADER

	mrb_state *mrb;

    ffi_closure *closure;
    ffi_cif *cif;
    
    int argc;
    mrb_value *arg_types;
    ffi_type **arg_ffi_types;
    mrb_value return_type;

    void *closure_pointer;

    int packed_args_size;
};

#define cfunc_closure_data_pointer(mrb, val) \
    cfunc_pointer_ptr(val)

void init_cfunc_closure(mrb_state *mrb, struct RClass* module);

mrb_value cfunc_closure_new(mrb_state *mrb, mrb_value self);
mrb_value cfunc_closure_to_pointer(mrb_state *mrb, mrb_value self);

void cfunc_closure_call_binding(ffi_cif *cif, void *ret, void **args, void *self_);

#endif
