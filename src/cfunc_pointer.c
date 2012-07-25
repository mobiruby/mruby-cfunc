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
        free(get_cfunc_pointer_data(data));
    }
    free(p);
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
    struct cfunc_type_data *data = malloc(sizeof(struct cfunc_type_data));
    data->refer = false;
    data->autofree = false;

    mrb_int alloc_size;
    mrb_get_args(mrb, "i", &alloc_size);
    
    set_cfunc_pointer_data(data, malloc(alloc_size));
    
    return mrb_obj_value(Data_Wrap_Struct(mrb, mrb_class_ptr(klass), &cfunc_pointer_data_type, data));
}


mrb_value
cfunc_pointer_new_with_pointer(mrb_state *mrb, void *p, bool autofree)
{
    struct cfunc_type_data *data = malloc(sizeof(struct cfunc_type_data));
    data->refer = false;
    data->autofree = autofree;

    set_cfunc_pointer_data(data, p);

    return mrb_obj_value(Data_Wrap_Struct(mrb, cfunc_state(mrb)->pointer_class, &cfunc_pointer_data_type, data));
}


mrb_value
cfunc_pointer_refer(mrb_state *mrb, mrb_value klass)
{
    struct RClass *c = mrb_class_ptr(klass);
    struct cfunc_type_data *data = malloc(sizeof(struct cfunc_type_data));
    data->refer = true;
    data->autofree = false;

    mrb_value pointer;
    mrb_get_args(mrb, "o", &pointer);

    data->value._pointer = mobi_pointer_ptr(pointer);

    struct RObject *obj = (struct RObject *)Data_Wrap_Struct(mrb, c, &cfunc_pointer_data_type, data);
    mrb_obj_iv_set(mrb, obj, mrb_intern(mrb, "parent_pointer"), pointer); // keep for GC
    return mrb_obj_value(obj);
}


mrb_value
cfunc_pointer_initialize(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data;
    data = mrb_get_datatype(mrb, self, &cfunc_pointer_data_type);
    if (!data) {
        data = malloc(sizeof(struct cfunc_type_data));
    }
    data->refer = false;
    data->autofree = false;
    DATA_PTR(self) = data;
    DATA_TYPE(self) = &cfunc_pointer_data_type;   

    mrb_value ptr;
    int argc = mrb_get_args(mrb, "|o", &ptr);
    if(argc == 0) {
        set_cfunc_pointer_data(data, NULL);
    }
    else {
        set_cfunc_pointer_data(data, mobi_pointer_ptr(ptr));
    }

    return self;
}


mrb_value
cfunc_pointer_realloc(mrb_state *mrb, mrb_value self)
{
    mrb_int alloc_size;
    struct cfunc_type_data *data = DATA_PTR(self);
    
    cfunc_pointer_destructor(mrb, data);
    mrb_get_args(mrb, "i", &alloc_size);
    set_cfunc_pointer_data(data, realloc(get_cfunc_pointer_data(data), alloc_size));
    
    return self;
}


mrb_value
cfunc_pointer_free(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    
    free(get_cfunc_pointer_data(data));
    data->autofree = false;
    set_cfunc_pointer_data(data, NULL);
    
    return self;
}


mrb_value
cfunc_pointer_inspect(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    char cstr[256];
    mrb_value str;
    
    const char* classname = mrb_obj_classname(mrb, self);
    if(!classname) {
        classname = "Unknown pointer";
    }
    snprintf(cstr, sizeof(cstr), "<%s=%p>", classname, get_cfunc_pointer_data(data));
    str = mrb_str_new2(mrb, cstr);
    
    return str;
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
    size_t len;
    mrb_value str;
    struct RString *s;
    const char* p = (const char*)get_cfunc_pointer_data(data);
        
    len = strlen(p);
    str = mrb_str_new(mrb, 0, len);
    s = mrb_str_ptr(str);
    strcpy(s->ptr, p);
    s->len = strlen(s->ptr);
    return str;
}


mrb_value
cfunc_pointer_offset(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    int offset;
    mrb_get_args(mrb, "i", &offset);

    mrb_value ptr = cfunc_pointer_new_with_pointer(mrb, (void*)((uint8_t*)get_cfunc_pointer_data(data) + offset), false);
    mrb_obj_iv_set(mrb, mrb_obj_ptr(ptr), mrb_intern(mrb, "parent_pointer"), self); // keep for GC
    return ptr;
}


mrb_value
cfunc_pointer_to_pointer(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = DATA_PTR(self);
    void *ptr = NULL;
    if(data->refer) {
        ptr = data->value._pointer;
    }
    else {
        ptr = &data->value._pointer;
    }

    mrb_value obj = cfunc_pointer_new_with_pointer(mrb, ptr, 0);
    mrb_obj_iv_set(mrb, mrb_obj_ptr(obj), mrb_intern(mrb, "parent_pointer"), self); // keep for GC
    return obj;
}


static mrb_value
cfunc_string_to_pointer(mrb_state *mrb, mrb_value self)
{
    mrb_value ptr = cfunc_pointer_new_with_pointer(mrb, &RSTRING_PTR(self), false);
    mrb_obj_iv_set(mrb, mrb_obj_ptr(ptr), mrb_intern(mrb, "parent_pointer"), self); // keep for GC
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
    *(void**)p = mobi_pointer_ptr(val);
}


static mrb_value
cfunc_pointer_data_to_mrb(mrb_state *mrb, struct cfunc_type_data *data)
{
    return cfunc_pointer_new_with_pointer(mrb, get_cfunc_pointer_data(data), false);
}


static void
cfunc_pointer_mrb_to_data(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data)
{
    set_cfunc_pointer_data(data, mobi_pointer_ptr(val));
}


static void
cfunc_pointer_ffi_data_destructor(mrb_state *mrb, void *p_)
{
    // ToDo: when *p_ was local scope variant?
};


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
    struct RClass *pointer_class = mrb_define_class_under(mrb, module, "Pointer", cfunc_state(mrb)->type_class);
    mrb_value ffi_type = mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &cfunc_pointer_ffi_data_type, &pointer_mrb_ffi_type));
    mrb_obj_iv_set(mrb, (struct RObject*)pointer_class, mrb_intern(mrb, "ffi_type"), ffi_type);
    cfunc_state(mrb)->pointer_class = pointer_class;

    mrb_define_class_method(mrb, pointer_class, "refer", cfunc_pointer_refer, ARGS_REQ(1));
    mrb_define_class_method(mrb, pointer_class, "malloc", cfunc_pointer_class_malloc, ARGS_REQ(1));

    mrb_define_method(mrb, pointer_class, "initialize", cfunc_pointer_initialize, ARGS_ANY());
    mrb_define_method(mrb, pointer_class, "realloc", cfunc_pointer_realloc, ARGS_REQ(1));
    mrb_define_method(mrb, pointer_class, "free", cfunc_pointer_free, ARGS_NONE());
    mrb_define_method(mrb, pointer_class, "inspect", cfunc_pointer_inspect, ARGS_NONE());
    mrb_define_method(mrb, pointer_class, "is_null?", cfunc_pointer_is_null, ARGS_NONE());
    mrb_define_method(mrb, pointer_class, "autofree", cfunc_pointer_autofree, ARGS_NONE());

    mrb_define_method(mrb, pointer_class, "offset", cfunc_pointer_offset, ARGS_REQ(1));
    mrb_define_method(mrb, pointer_class, "to_s", cfunc_pointer_to_s, ARGS_NONE());
    
    // add method to system classes
    mrb_define_method(mrb, mrb->string_class, "to_pointer", cfunc_string_to_pointer, ARGS_NONE());
    mrb_obj_iv_set(mrb, (struct RObject *)mrb->string_class, mrb_intern(mrb, "ffi_type"), ffi_type);
}
