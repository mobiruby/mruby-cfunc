//
//  CFunc::Struct class
// 
//  See Copyright Notice in cfunc.h
//


#ifndef cfunc_struct_h
#define cfunc_struct_h

#include "cfunc.h"
#include "ffi.h"

struct cfunc_struct_data {
    ffi_type *type;
    void *buf;
    int bufsize;
};
extern const struct mrb_data_type cfunc_struct_data_type;

mrb_value cfunc_struct_define_struct(mrb_state *mrb, mrb_value self);

void init_cfunc_struct(mrb_state *mrb, struct RClass* module);

#endif
