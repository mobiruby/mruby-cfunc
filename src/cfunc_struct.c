//
//  CFunc::Struct class
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc_struct.h"
#include "cfunc_type.h"
#include "cfunc_pointer.h"

#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "ffi.h"

#include <stdio.h>
#include <stdbool.h>

static void
cfunc_struct_data_destructor(mrb_state *mrb, void *p_)
{
    // todo
};


const struct mrb_data_type cfunc_struct_data_type = {
    "cfunc_struct", cfunc_struct_data_destructor,
};


static mrb_value
cfunc_type_ffi_struct_c_to_mrb(mrb_state *mrb, void *p)
{
    // todo
    return mrb_nil_value();
}


static void
cfunc_type_ffi_struct_mrb_to_c(mrb_state *mrb, mrb_value val, void *p)
{
    // todo
}


mrb_value
cfunc_struct_define_struct(mrb_state *mrb, mrb_value klass)
{
    mrb_value elements_mrb;
    mrb_get_args(mrb, "A", &elements_mrb);
    struct RArray *elements = mrb_ary_ptr(elements_mrb);

    ffi_type *tm_type = malloc(sizeof(ffi_type));
    tm_type->type = FFI_TYPE_STRUCT;
    tm_type->size = tm_type->alignment = 0;

    ffi_type **tm_type_elements = malloc(sizeof(ffi_type*) * (elements->len + 1));
    int i;
    for(i = 0; i < elements->len; ++i) {
        tm_type_elements[i] = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(elements->ptr[i]))->ffi_type_value;
    }
    tm_type_elements[i] = NULL;
    tm_type->elements = tm_type_elements;

    struct mrb_ffi_type *mft = malloc(sizeof(struct mrb_ffi_type));
    mft->name = mrb_class_name(mrb, mrb_class_ptr(klass));
    mft->ffi_type_value = tm_type;
    mft->mrb_to_c = &cfunc_type_ffi_struct_mrb_to_c;
    mft->c_to_mrb = &cfunc_type_ffi_struct_c_to_mrb;

    mrb_value __ffi_type = mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &cfunc_struct_data_type, mft));
    mrb_obj_iv_set(mrb, (struct RObject*)(mrb_class_ptr(klass)), mrb_intern(mrb, "ffi_type"), __ffi_type);
    
    return mrb_nil_value();
}


void
init_cfunc_struct(mrb_state *mrb, struct RClass* module)
{
    struct RClass *struct_class = mrb_define_class_under(mrb, module, "Struct", mrb->object_class);
    cfunc_state(mrb)->struct_class = struct_class;
    
    mrb_define_class_method(mrb, struct_class, "define_struct", cfunc_struct_define_struct, ARGS_REQ(1));
}
