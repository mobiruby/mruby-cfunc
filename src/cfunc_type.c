//
//  CFunc Basic types
// 
//  See Copyright Notice in cfunc.h
//


#include "cfunc_type.h"
#include "cfunc_pointer.h"

#include "mruby/array.h"
#include "mruby/data.h"
#include "mruby/string.h"
#include "mruby/class.h"
#include "mruby/variable.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>


static void
cfunc_type_destructor(mrb_state *mrb, void *p)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)p;
    if(data->autofree) {
        free(data->value._pointer);
    }
    mrb_free(mrb, p);
}

static struct mrb_data_type cfunc_type_data = {
    "cfunc_type_data", cfunc_type_destructor
};



struct mrb_ffi_type*
rclass_to_mrb_ffi_type(mrb_state *mrb, struct RClass *cls)
{
    while(cls) {
        mrb_value ffi_type = mrb_obj_iv_get(mrb, (struct RObject*)cls, mrb_intern(mrb, "ffi_type"));
        if(mrb_test(ffi_type)) {
            return (struct mrb_ffi_type*)DATA_PTR(ffi_type);
        }
        cls = cls->super;
    }
    mrb_raise(mrb, E_TYPE_ERROR, "Cannot convert to c value");
    return NULL;
}


struct mrb_ffi_type*
mrb_value_to_mrb_ffi_type(mrb_state *mrb, mrb_value val)
{
    return rclass_to_mrb_ffi_type(mrb, RBASIC_KLASS(val));
}


static mrb_value
cfunc_type_class_refer(mrb_state *mrb, mrb_value klass)
{
    struct RClass *c = mrb_class_ptr(klass);
    struct cfunc_type_data *data = malloc(sizeof(struct cfunc_type_data));
    data->autofree = false;

    mrb_value pointer;
    mrb_get_args(mrb, "o", &pointer);

    data->refer = true;
    data->value._pointer = cfunc_pointer_ptr(pointer);

    struct RObject *obj = (struct RObject *)Data_Wrap_Struct(mrb, c, &cfunc_type_data, data);
    mrb_obj_iv_set(mrb, obj, mrb_intern(mrb, "parent_pointer"), pointer); // keep for GC
    return mrb_obj_value(obj);
}


static mrb_value
cfunc_type_initialize(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data;
    data = mrb_get_datatype(mrb, self, &cfunc_type_data);
    if (!data) {
        data = malloc(sizeof(struct cfunc_type_data));
        data->value._uint64 = 0;
    }
    data->autofree = false;
    data->refer = false;
    DATA_PTR(self) = data;
    DATA_TYPE(self) = &cfunc_type_data;   

    mrb_value val;
    int argc = mrb_get_args(mrb, "|o", &val);
    if(argc > 0) {
        struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, RBASIC_KLASS(self));
        mft->mrb_to_data(mrb, val, data);
    }

    return self;
}


static mrb_value
cfunc_type_size(mrb_state *mrb, mrb_value klass)
{
    struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(klass));
    return mrb_fixnum_value(mft->ffi_type_value->size);
}


static mrb_value
cfunc_type_align(mrb_state *mrb, mrb_value klass)
{
    struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(klass));
    return mrb_fixnum_value(mft->ffi_type_value->alignment);
}


mrb_value
cfunc_type_class_get(mrb_state *mrb, mrb_value klass)
{
    mrb_value pointer;
    mrb_get_args(mrb, "o", &pointer);

    struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(klass));
    return mft->c_to_mrb(mrb, cfunc_pointer_ptr(pointer));
}


mrb_value
cfunc_type_class_set(mrb_state *mrb, mrb_value klass)
{
    mrb_value pointer, val;
    mrb_get_args(mrb, "oo", &pointer, &val);

    struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(klass));
    mft->mrb_to_c(mrb, val, cfunc_pointer_ptr(pointer));

    return val;
}


mrb_value
cfunc_type_get_value(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, RBASIC_KLASS(self));
    return mft->data_to_mrb(mrb, data);
}


mrb_value
cfunc_type_set_value(mrb_state *mrb, mrb_value self)
{
    mrb_value val;
    mrb_get_args(mrb, "o", &val);

    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, RBASIC_KLASS(self));
    mft->mrb_to_data(mrb, val, data);

    return val;
}


mrb_value
cfunc_type_to_pointer(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);

    mrb_value ptr;
    if(data->refer) {
        ptr = cfunc_pointer_new_with_pointer(mrb, data->value._pointer, false);
    }
    else {
        ptr = cfunc_pointer_new_with_pointer(mrb, &data->value._pointer, false);
    }

    mrb_obj_iv_set(mrb, mrb_obj_ptr(ptr), mrb_intern(mrb, "parent_pointer"), self); // keep for GC

    return ptr;
}


static mrb_value
cfunc_type_ffi_void_data_to_mrb(mrb_state *mrb, struct cfunc_type_data *data)
{
    return mrb_nil_value();
}


static void
cfunc_type_ffi_void_mrb_to_data(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data)
{
    // NO OP
}


static mrb_value
cfunc_type_ffi_void_c_to_mrb(mrb_state *mrb, void *p)
{
    return mrb_nil_value();
}


static void
cfunc_type_ffi_void_mrb_to_c(mrb_state *mrb, mrb_value val, void *p)
{
    // NO OP
}


#define define_cfunc_type(name, ffi_type_ptr, ctype, c_to_mrb, mrb_to_c) \
\
static mrb_value \
cfunc_type_ffi_##name##_c_to_mrb(mrb_state *mrb, void *p) \
{ \
    return c_to_mrb(*(ctype*)p); \
} \
\
static void \
cfunc_type_ffi_##name##_mrb_to_c(mrb_state *mrb, mrb_value val, void *p) \
{ \
    *(ctype*)p = mrb_to_c(val); \
} \
\
static mrb_value \
cfunc_type_ffi_##name##_data_to_mrb(mrb_state *mrb, struct cfunc_type_data *data) \
{ \
    if(data->refer) { \
        return c_to_mrb(*(ctype*)data->value._pointer); \
    } \
    else { \
        return c_to_mrb(data->value._##name); \
    } \
} \
\
static void \
cfunc_type_ffi_##name##_mrb_to_data(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data) \
{ \
    if(data->refer) { \
        *(ctype*)data->value._pointer = mrb_to_c(val); \
    } \
    else { \
        data->value._##name = mrb_to_c(val); \
    } \
}

#define define_mrb_ffi_type(name_, type_) \
{ \
    .name = #name_, \
    .ffi_type_value = &ffi_type_##type_, \
    .mrb_to_c = &cfunc_type_ffi_##type_##_mrb_to_c, \
    .c_to_mrb = &cfunc_type_ffi_##type_##_c_to_mrb, \
    .mrb_to_data = &cfunc_type_ffi_##type_##_mrb_to_data, \
    .data_to_mrb = &cfunc_type_ffi_##type_##_data_to_mrb \
}

define_cfunc_type(sint8, &ffi_type_sint8, int8_t, mrb_fixnum_value, mrb_fixnum);
define_cfunc_type(uint8, &ffi_type_uint8, uint8_t, mrb_fixnum_value, mrb_fixnum);

define_cfunc_type(sint16, &ffi_type_sint16, int16_t, mrb_fixnum_value, mrb_fixnum);
define_cfunc_type(uint16, &ffi_type_uint16, uint16_t, mrb_fixnum_value, mrb_fixnum);

define_cfunc_type(sint32, &ffi_type_sint32, int32_t, mrb_fixnum_value, mrb_fixnum);
define_cfunc_type(uint32, &ffi_type_uint32, uint32_t, mrb_fixnum_value, mrb_fixnum);

define_cfunc_type(sint64, &ffi_type_sint64, int64_t, mrb_fixnum_value, mrb_fixnum);
define_cfunc_type(uint64, &ffi_type_uint64, uint64_t, mrb_fixnum_value, mrb_fixnum);

define_cfunc_type(float, &ffi_type_float, float, mrb_float_value, mrb_float);
define_cfunc_type(double, &ffi_type_double, double, mrb_float_value, mrb_float);


static struct mrb_ffi_type types[] = {
    define_mrb_ffi_type(Void, void),
    define_mrb_ffi_type(UInt8, uint8),
    define_mrb_ffi_type(SInt8, sint8),
    define_mrb_ffi_type(UInt16, uint16),
    define_mrb_ffi_type(SInt16, sint16),
    define_mrb_ffi_type(UInt32, uint32),
    define_mrb_ffi_type(SInt32, sint32),
    define_mrb_ffi_type(UInt64, uint64),
    define_mrb_ffi_type(SInt64, sint64),
    define_mrb_ffi_type(Float, float),
    define_mrb_ffi_type(Double, double)
};

static void
cfunc_class_ffi_destructor(mrb_state *mrb, void *p)
{
}

const struct mrb_data_type cfunc_class_ffi_data_type = {
    "cfunc_class_ffi", cfunc_class_ffi_destructor,
};


void init_cfunc_type(mrb_state *mrb, struct RClass* module)
{
    struct RClass *type_class = mrb_define_class_under(mrb, module, "Type", mrb->object_class);
    MRB_SET_INSTANCE_TT(type_class, MRB_TT_DATA);
    cfunc_state(mrb)->type_class = type_class;

    mrb_define_class_method(mrb, type_class, "refer", cfunc_type_class_refer, ARGS_REQ(1));
    mrb_define_class_method(mrb, type_class, "size", cfunc_type_size, ARGS_NONE());
    mrb_define_class_method(mrb, type_class, "align", cfunc_type_align, ARGS_NONE());
    mrb_define_class_method(mrb, type_class, "get", cfunc_type_class_get, ARGS_REQ(1));
    mrb_define_class_method(mrb, type_class, "set", cfunc_type_class_set, ARGS_REQ(2));

    mrb_define_method(mrb, type_class, "initialize", cfunc_type_initialize, ARGS_ANY());
    mrb_define_method(mrb, type_class, "value", cfunc_type_get_value, ARGS_NONE());
    mrb_define_method(mrb, type_class, "value=", cfunc_type_set_value, ARGS_REQ(1));
    mrb_define_method(mrb, type_class, "to_pointer", cfunc_type_to_pointer, ARGS_NONE());

    int map_size = sizeof(types) / sizeof(struct mrb_ffi_type);
    for(int i = 0; i < map_size; ++i) {
        struct RClass *new_class = mrb_define_class_under(mrb, module, types[i].name, type_class);
        mrb_value ffi_type = mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &cfunc_class_ffi_data_type, &types[i]));
        mrb_obj_iv_set(mrb, (struct RObject*)new_class, mrb_intern(mrb, "ffi_type"), ffi_type);
    }
    
    mrb_value mod = mrb_obj_value(module);
    cfunc_state(mrb)->void_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "Void")));
    cfunc_state(mrb)->uint8_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "UInt8")));
    cfunc_state(mrb)->sint8_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "SInt8")));
    cfunc_state(mrb)->uint16_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "UInt16")));
    cfunc_state(mrb)->sint16_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "SInt16")));
    cfunc_state(mrb)->uint32_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "UInt32")));
    cfunc_state(mrb)->sint32_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "SInt32")));
    cfunc_state(mrb)->uint64_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "UInt64")));
    cfunc_state(mrb)->sint64_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "SInt64")));
    cfunc_state(mrb)->float_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "Float")));
    cfunc_state(mrb)->double_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern(mrb, "Double")));
}
