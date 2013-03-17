//
//  CFunc::Closure class
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc_closure.h"
#include "cfunc_type.h"
#include "cfunc_pointer.h"

#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/string.h"
#include "mruby/array.h"
#include "mruby/variable.h"

#include "ffi.h"

#include <stdio.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>


static void
cfunc_closure_destructor(mrb_state *mrb, void *p_)
{
    struct cfunc_closure_data *p = p_;
    if (p->closure) {
        ffi_closure_free(p->closure);
    }
    mrb_free(mrb, p->arg_types);
    mrb_free(mrb, p->arg_ffi_types);
    mrb_free(mrb, p->cif);
    mrb_free(mrb, p);
}

static void
cfunc_closure_ffi_type_destructor(mrb_state *mrb, void *p_)
{
    // Nothing
}

static struct mrb_data_type cfunc_closure_data_type = {
    "cfunc_closure", cfunc_closure_destructor,
};


static struct mrb_data_type cfunc_closure_ffi_type_data_type = {
    "cfunc_closure_ffi_type", cfunc_closure_ffi_type_destructor,
};


mrb_value
cfunc_closure_initialize(mrb_state *mrb, mrb_value self)
{
    struct cfunc_closure_data *data;
    data = mrb_get_datatype(mrb, self, &cfunc_closure_data_type);
    if (!data) {
        data = mrb_malloc(mrb, sizeof(struct cfunc_closure_data));
    }
    data->refer = 0;
    data->autofree = 0;
    DATA_PTR(self) = data;
    DATA_TYPE(self) = &cfunc_closure_data_type;

    data->mrb = mrb;
    data->closure = NULL;
    data->arg_types = NULL;
    
    mrb_value rettype_mrb, block, args_mrb;
    mrb_get_args(mrb, "&oo", &block, &rettype_mrb, &args_mrb);
    data->argc = RARRAY_LEN(args_mrb);
    
    ffi_type *return_ffi_type = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(rettype_mrb))->ffi_type_value;
    data->return_type = rettype_mrb;

    data->arg_ffi_types = mrb_malloc(mrb, sizeof(ffi_type*) * data->argc);
    data->arg_types = mrb_malloc(mrb, sizeof(mrb_value) * data->argc);
    int i;
    for (i = 0; i < data->argc; ++i) {
        data->arg_types[i] = mrb_ary_ref(mrb, args_mrb, i);
        data->arg_ffi_types[i] = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(data->arg_types[i]))->ffi_type_value;
    }
    
    mrb_iv_set(mrb, self, mrb_intern(data->mrb, "@block"), block);

    void *closure_pointer = NULL;
    data->closure = ffi_closure_alloc(sizeof(ffi_closure) + sizeof(void*), &closure_pointer);
    data->cif = mrb_malloc(mrb, sizeof(ffi_cif));
    
    if (data->closure) {
        if (ffi_prep_cif(data->cif, FFI_DEFAULT_ABI, data->argc, return_ffi_type, data->arg_ffi_types) == FFI_OK) {
            if (ffi_prep_closure_loc(data->closure, data->cif, cfunc_closure_call_binding, mrb_object(self), closure_pointer) == FFI_OK) {
                set_cfunc_pointer_data((struct cfunc_type_data *)data, closure_pointer);
                return self;
            }
        }
    }

    mrb_raise(mrb, E_SCRIPT_ERROR, "Internal FFI call error");
    return mrb_nil_value();
}


void
cfunc_closure_call_binding(ffi_cif *cif, void *ret, void **args, void *self_)
{
    mrb_value self = mrb_obj_value(self_);
    struct cfunc_closure_data *data = DATA_PTR(self);

    int ai = mrb_gc_arena_save(data->mrb);

    mrb_value *ary = mrb_malloc(data->mrb, sizeof(mrb_value) * data->argc);
    int i;
    for (i = 0; i < data->argc; ++i) {
        // TODO: I felt too much consume memory
        void *p = mrb_malloc(data->mrb, data->arg_ffi_types[i]->size);
        memcpy(p, args[i], data->arg_ffi_types[i]->size);
        mrb_value pointer = cfunc_pointer_new_with_pointer(data->mrb, p, true);
        ary[i] = mrb_funcall(data->mrb, data->arg_types[i], "refer", 1, pointer);
    }

    mrb_value block = mrb_iv_get(data->mrb, self, mrb_intern(data->mrb, "@block"));
    mrb_value result = mrb_funcall_argv(data->mrb, block, mrb_intern(data->mrb, "call"), data->argc, ary);
    mrb_free(data->mrb, ary);

    mrb_value ret_pointer = cfunc_pointer_new_with_pointer(data->mrb, ret, false);
    mrb_funcall(data->mrb, data->return_type, "set", 2, ret_pointer, result);
    
    mrb_gc_arena_restore(data->mrb, ai);
}


static mrb_value
cfunc_closure_c_to_mrb(mrb_state *mrb, void* p)
{
    return cfunc_pointer_new_with_pointer(mrb, *(void**)p, false);
}


static void
cfunc_closure_mrb_to_c(mrb_state *mrb, mrb_value val, void *p)
{
    *(void**)p = cfunc_pointer_ptr(val);
}


static mrb_value
cfunc_closure_data_to_mrb(mrb_state *mrb, struct cfunc_type_data *data)
{
    return cfunc_pointer_new_with_pointer(mrb, get_cfunc_pointer_data(data), false);
}


static void
cfunc_closure_mrb_to_data(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data)
{
    set_cfunc_pointer_data(data, cfunc_pointer_ptr(val));
}


static void
cfunc_closure_ffi_data_destructor(mrb_state *mrb, void *p_)
{
    // ToDo:
};


static struct mrb_ffi_type closure_mrb_ffi_type = {
    .name = "Closure",
    .ffi_type_value = &ffi_type_pointer,
    .mrb_to_c = &cfunc_closure_mrb_to_c,
    .c_to_mrb = &cfunc_closure_c_to_mrb,
    .mrb_to_data = &cfunc_closure_mrb_to_data,
    .data_to_mrb = &cfunc_closure_data_to_mrb
};


void
init_cfunc_closure(mrb_state *mrb, struct RClass* module)
{
    struct cfunc_state *state = cfunc_state(mrb, module);
    struct RClass *closure_class = mrb_define_class_under(mrb, module, "Closure", state->pointer_class);
    state->closure_class = closure_class;

    mrb_value ffi_type = mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &cfunc_closure_ffi_type_data_type, &closure_mrb_ffi_type));
    mrb_obj_iv_set(mrb, (struct RObject*)closure_class, mrb_intern(mrb, "@ffi_type"), ffi_type);

    mrb_define_method(mrb, closure_class, "initialize", cfunc_closure_initialize, ARGS_ANY());
}
