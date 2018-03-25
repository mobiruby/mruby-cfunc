//
//  CFunc::Pointer class
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc_pointer.h"
#include "cfunc_type.h"
#include "cfunc_call.h"

#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "ffi.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>



static void
cfunc_pointer_destructor(mrb_state *mrb, void *p)
{
    struct cfunc_type_data *data = (struct cfunc_type_data *)p;
    if(data->autofree) {
        mrb_free(mrb, get_cfunc_pointer_data(data));
    }
    mrb_free(mrb, p);
}


static struct mrb_data_type cfunc_pointer_data_type = {
    "cfunc_pointer", cfunc_pointer_destructor,
};


void* get_cfunc_pointer_data(struct cfunc_type_data *data)
{
    if(data->refer) {
        return *(void**)data->value._pointer;
    }
    else {
        return data->value._pointer;
    }
}


void set_cfunc_pointer_data(struct cfunc_type_data *data, void *p)
{
    if(data->refer) {
        *(void**)data->value._pointer = p;
    }
    else {
        data->value._pointer = p;
    }
}


mrb_value
cfunc_pointer_class_malloc(mrb_state *mrb, mrb_value klass)
{
    struct cfunc_type_data *data = mrb_malloc(mrb, sizeof(struct cfunc_type_data));
    mrb_int alloc_size;
    data->refer = false;
    data->autofree = false;

    mrb_get_args(mrb, "i", &alloc_size);
    
    set_cfunc_pointer_data(data, mrb_malloc(mrb, alloc_size));
    data->autofree = true;
    
    return mrb_obj_value(Data_Wrap_Struct(mrb, mrb_class_ptr(klass), &cfunc_pointer_data_type, data));
}


mrb_value
cfunc_pointer_new_with_pointer(mrb_state *mrb, void *p, bool autofree)
{
    struct cfunc_type_data *data = mrb_malloc(mrb, sizeof(struct cfunc_type_data));
    struct cfunc_state *state;
    data->refer = false;
    data->autofree = autofree;

    set_cfunc_pointer_data(data, p);

    state = cfunc_state(mrb, NULL);
    return mrb_obj_value(Data_Wrap_Struct(mrb, state->pointer_class, &cfunc_pointer_data_type, data));
}


mrb_value
cfunc_pointer_refer(mrb_state *mrb, mrb_value klass)
{
    struct RClass *c = mrb_class_ptr(klass);
    struct cfunc_type_data *data = mrb_malloc(mrb, sizeof(struct cfunc_type_data));
    mrb_value pointer;
    struct RObject *obj;
    data->refer = true;
    data->autofree = false;

    mrb_get_args(mrb, "o", &pointer);

    data->value._pointer = cfunc_pointer_ptr(pointer);

    obj = (struct RObject *)Data_Wrap_Struct(mrb, c, &cfunc_pointer_data_type, data);
    mrb_obj_iv_set(mrb, obj, mrb_intern_lit(mrb, "parent_pointer"), pointer); // keep for GC
    return mrb_obj_value(obj);
}


mrb_value
cfunc_pointer_initialize(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data;
    mrb_value ptr;
    int argc;
    data = mrb_data_check_get_ptr(mrb, self, &cfunc_pointer_data_type);
    if(!data) {
        data = mrb_malloc(mrb, sizeof(struct cfunc_type_data));
        DATA_PTR(self) = data;
        DATA_TYPE(self) = &cfunc_pointer_data_type;   
    }
    data->refer = false;
    data->autofree = false;

    argc = mrb_get_args(mrb, "|o", &ptr);
    if(argc == 0) {
        set_cfunc_pointer_data(data, NULL);
    }
    else {
        set_cfunc_pointer_data(data, cfunc_pointer_ptr(ptr));
    }

    return self;
}


mrb_value
cfunc_pointer_realloc(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    
    mrb_int alloc_size;
    mrb_get_args(mrb, "i", &alloc_size);
    set_cfunc_pointer_data(data, mrb_realloc(mrb, get_cfunc_pointer_data(data), alloc_size));
    
    return self;
}


mrb_value
cfunc_pointer_free(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    
    mrb_free(mrb, get_cfunc_pointer_data(data));
    data->autofree = false;
    set_cfunc_pointer_data(data, NULL);
    
    return self;
}


mrb_value
cfunc_pointer_inspect(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    char cstr[256];
    
    mrb_value type = mrb_funcall(mrb, mrb_obj_value(mrb_class(mrb, self)), "type", 0);
    const char* classname = mrb_class_name(mrb, mrb_class_ptr(type));
    if(!classname) {
        classname = "Unknown pointer";
    }

    snprintf(cstr, sizeof(cstr), "<%s pointer=%p>", classname, get_cfunc_pointer_data(data));
    
    return mrb_str_new_cstr(mrb, cstr);
}


mrb_value
cfunc_pointer_is_null(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    return (get_cfunc_pointer_data(data) == NULL) ? mrb_true_value() : mrb_false_value();
}


mrb_value
cfunc_pointer_autofree(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    data->autofree = true;
    return self;
}


mrb_value
cfunc_pointer_to_s(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    const char* p = (const char*)get_cfunc_pointer_data(data);
    return mrb_str_new_cstr(mrb, p);
}


mrb_value
cfunc_pointer_offset(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    mrb_int offset;
    mrb_get_args(mrb, "i", &offset);

    if(offset == 0) {
        return self;
    }
    else {
        mrb_value ptr = cfunc_pointer_new_with_pointer(mrb, (void*)((uint8_t*)get_cfunc_pointer_data(data) + offset), false);
        mrb_obj_iv_set(mrb, mrb_obj_ptr(ptr), mrb_intern_lit(mrb, "parent_pointer"), self); // keep for GC
        return ptr;
    }
}


mrb_value
cfunc_pointer_addr(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    void *ptr = NULL;
    mrb_value obj;
    if(data->refer) {
        ptr = data->value._pointer;
    }
    else {
        ptr = &data->value._pointer;
    }

    obj = cfunc_pointer_new_with_pointer(mrb, ptr, false);
    mrb_obj_iv_set(mrb, mrb_obj_ptr(obj), mrb_intern_lit(mrb, "parent_pointer"), self); // keep for GC
    return obj;
}


static mrb_value
cfunc_string_addr(mrb_state *mrb, mrb_value self)
{
    mrb_value ptr;
    // move string to heap
    mrb_str_modify(mrb, RSTRING(self));
    if (RSTR_EMBED_P(RSTRING(self))) {
        mrb_int const tmp_s = RSTRING_LEN(self);
        mrb_str_resize(mrb, self, RSTRING_EMBED_LEN_MAX + 1);
        mrb_str_resize(mrb, self, tmp_s);
        mrb_assert(!RSTR_EMBED_P(RSTRING(self)));
    }

    ptr = cfunc_pointer_new_with_pointer(mrb, &RSTRING(self)->as.heap.ptr, false);
    mrb_obj_iv_set(mrb, mrb_obj_ptr(ptr), mrb_intern_lit(mrb, "parent_pointer"), self); // keep for GC
    return ptr;
}


static mrb_value
cfunc_pointer_c_to_mrb(mrb_state *mrb, void* p)
{
    return cfunc_pointer_new_with_pointer(mrb, *(void**)p, false);
}


static void
cfunc_pointer_mrb_to_c(mrb_state *mrb, mrb_value val, void *p)
{
    if(mrb_nil_p(val)) {
        *(void**)p = NULL;
    }
    else {
        *(void**)p = cfunc_pointer_ptr(val);
    }
}


static mrb_value
cfunc_pointer_data_to_mrb(mrb_state *mrb, struct cfunc_type_data *data)
{
    return cfunc_pointer_new_with_pointer(mrb, get_cfunc_pointer_data(data), false);
}


static void
cfunc_pointer_mrb_to_data(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data)
{
    set_cfunc_pointer_data(data, cfunc_pointer_ptr(val));
}


static void
cfunc_pointer_ffi_data_destructor(mrb_state *mrb, void *p_)
{
    // ToDo: when *p_ was local scope variant?
}


const struct mrb_data_type cfunc_pointer_ffi_data_type = {
    "cfunc_pointer", cfunc_pointer_ffi_data_destructor,
};


static struct mrb_ffi_type pointer_mrb_ffi_type = {
    .name = "Pointer",
    .ffi_type_value = &ffi_type_pointer,
    .mrb_to_c = &cfunc_pointer_mrb_to_c,
    .c_to_mrb = &cfunc_pointer_c_to_mrb,
    .mrb_to_data = &cfunc_pointer_mrb_to_data,
    .data_to_mrb = &cfunc_pointer_data_to_mrb
};


void
init_cfunc_pointer(mrb_state *mrb, struct RClass* module)
{
    struct cfunc_state *state = cfunc_state(mrb, module);

    struct RClass *pointer_class = mrb_define_class_under(mrb, module, "Pointer", state->type_class);
    mrb_value ffi_type = mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &cfunc_pointer_ffi_data_type, &pointer_mrb_ffi_type));
    mrb_obj_iv_set(mrb, (struct RObject*)pointer_class, mrb_intern_lit(mrb, "@ffi_type"), ffi_type);
    state->pointer_class = pointer_class;

    mrb_define_class_method(mrb, pointer_class, "refer", cfunc_pointer_refer, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, pointer_class, "malloc", cfunc_pointer_class_malloc, MRB_ARGS_REQ(1));

    mrb_define_method(mrb, pointer_class, "initialize", cfunc_pointer_initialize, MRB_ARGS_ANY());
    mrb_define_method(mrb, pointer_class, "realloc", cfunc_pointer_realloc, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pointer_class, "free", cfunc_pointer_free, MRB_ARGS_NONE());
    mrb_define_method(mrb, pointer_class, "inspect", cfunc_pointer_inspect, MRB_ARGS_NONE());
    mrb_define_method(mrb, pointer_class, "is_null?", cfunc_pointer_is_null, MRB_ARGS_NONE());
    mrb_define_method(mrb, pointer_class, "autofree", cfunc_pointer_autofree, MRB_ARGS_NONE());

    mrb_define_method(mrb, pointer_class, "offset", cfunc_pointer_offset, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pointer_class, "to_s", cfunc_pointer_to_s, MRB_ARGS_NONE());
    
    // add method to system classes
    mrb_define_method(mrb, mrb->string_class, "addr", cfunc_string_addr, MRB_ARGS_NONE());
    mrb_obj_iv_set(mrb, (struct RObject *)mrb->string_class, mrb_intern_lit(mrb, "@ffi_type"), ffi_type);
}
