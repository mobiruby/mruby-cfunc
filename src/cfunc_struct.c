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
cfunc_struct_data_destructor(mrb_state *mrb, void *p)
{
    struct mrb_ffi_type *mft = (struct mrb_ffi_type*)p;
    mrb_free(mrb, mft->ffi_type_value->elements);
    mrb_free(mrb, mft->ffi_type_value);
    mrb_free(mrb, p);
}


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
    ffi_type *tm_type;
    ffi_type **tm_type_elements;
    int i;
    struct mrb_ffi_type *mft;
    mrb_value __ffi_type;
    mrb_get_args(mrb, "A", &elements_mrb);

    tm_type = (ffi_type*)mrb_malloc(mrb, sizeof(ffi_type));
    tm_type->type = FFI_TYPE_STRUCT;
    tm_type->size = tm_type->alignment = 0;

    tm_type_elements = (ffi_type**)mrb_malloc(mrb, sizeof(ffi_type*) * (RARRAY_LEN(elements_mrb) + 1));
    for(i = 0; i < RARRAY_LEN(elements_mrb); ++i) {
        tm_type_elements[i] = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(RARRAY_PTR(elements_mrb)[i]))->ffi_type_value;
    }
    tm_type_elements[i] = NULL;
    tm_type->elements = tm_type_elements;

    mft = (struct mrb_ffi_type*)mrb_malloc(mrb, sizeof(struct mrb_ffi_type));
    mft->name = mrb_class_name(mrb, mrb_class_ptr(klass));
    mft->ffi_type_value = tm_type;
    mft->mrb_to_c = &cfunc_type_ffi_struct_mrb_to_c;
    mft->c_to_mrb = &cfunc_type_ffi_struct_c_to_mrb;

    __ffi_type = mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &cfunc_struct_data_type, mft));
    mrb_obj_iv_set(mrb, (struct RObject*)(mrb_class_ptr(klass)), mrb_intern_lit(mrb, "@ffi_type"), __ffi_type);
    
    return mrb_nil_value();
}


void
init_cfunc_struct(mrb_state *mrb, struct RClass* module)
{
    struct cfunc_state *state = cfunc_state(mrb, module);
    struct RClass *struct_class = mrb_define_class_under(mrb, module, "Struct", mrb->object_class);
    set_cfunc_state(mrb, struct_class, state);
    state->struct_class = struct_class;
    
    mrb_define_class_method(mrb, struct_class, "define_struct", cfunc_struct_define_struct, MRB_ARGS_REQ(1));
}
