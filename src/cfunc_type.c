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

#include "ffi.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#define DONE mrb_gc_arena_restore(mrb, ai);

static void
cfunc_type_destructor(mrb_state *mrb, void *p)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)p;
    if(data->autofree) {
        mrb_free(mrb, data->value._pointer);
    }
    mrb_free(mrb, p);
}

static struct mrb_data_type cfunc_type_data = {
    "cfunc_type_data", cfunc_type_destructor
};



struct mrb_ffi_type*
rclass_to_mrb_ffi_type(mrb_state *mrb, struct RClass *cls)
{
    struct RClass *cls_ = cls;
    while(cls) {
        mrb_value ffi_type = mrb_obj_iv_get(mrb, (struct RObject*)cls, mrb_intern_lit(mrb, "@ffi_type"));
        if(mrb_test(ffi_type)) {
            return (struct mrb_ffi_type*)DATA_PTR(ffi_type);
        }
        cls = cls->super;
    }
    mrb_raisef(mrb, E_TYPE_ERROR, "%S cannot convert to C value", mrb_class_path(mrb, cls_));
    return NULL;
}


struct mrb_ffi_type*
mrb_value_to_mrb_ffi_type(mrb_state *mrb, mrb_value val)
{
    if(mrb_nil_p(val)) {
        return rclass_to_mrb_ffi_type(mrb, cfunc_state(mrb, NULL)->pointer_class);
    }
    switch(mrb_type(val)) {
        case MRB_TT_TRUE:
        case MRB_TT_FALSE:
            return rclass_to_mrb_ffi_type(mrb, cfunc_state(mrb, NULL)->sint32_class);
        default:
            return rclass_to_mrb_ffi_type(mrb, mrb_class(mrb, val));
    }
}


static mrb_value
cfunc_type_class_refer(mrb_state *mrb, mrb_value klass)
{
    struct RClass *c = mrb_class_ptr(klass);
    struct cfunc_type_data *data = (struct cfunc_type_data*)mrb_malloc(mrb, sizeof(struct cfunc_type_data));
    mrb_value pointer;
    struct RObject *obj;
    data->autofree = false;

    mrb_get_args(mrb, "o", &pointer);

    data->refer = true;
    data->value._pointer = cfunc_pointer_ptr(pointer);

    obj = (struct RObject *)Data_Wrap_Struct(mrb, c, &cfunc_type_data, data);
    mrb_obj_iv_set(mrb, obj, mrb_intern_lit(mrb, "parent_pointer"), pointer); // keep for GC
    return mrb_obj_value(obj);
}


static mrb_value
cfunc_type_initialize(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data;
    mrb_value val;
    int argc;
    data = (struct cfunc_type_data*)mrb_data_check_get_ptr(mrb, self, &cfunc_type_data);
    if (!data) {
        data = (struct cfunc_type_data*)mrb_malloc(mrb, sizeof(struct cfunc_type_data));
        data->value._uint64 = 0;
    }
    data->autofree = false;
    data->refer = false;
    DATA_PTR(self) = data;
    DATA_TYPE(self) = &cfunc_type_data;   

    argc = mrb_get_args(mrb, "|o", &val);
    if(argc > 0) {
        struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, mrb_class(mrb, self));
        if(mft && mft->mrb_to_data) {
            mft->mrb_to_data(mrb, val, data);
        }
        else {
            mrb_raise(mrb, E_TYPE_ERROR, "Fatal in cfunc_type_initialize");
        }
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
    struct mrb_ffi_type *mft;
    mrb_get_args(mrb, "o", &pointer);

    mft = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(klass));
    return mft->c_to_mrb(mrb, cfunc_pointer_ptr(pointer));
}


mrb_value
cfunc_type_class_set(mrb_state *mrb, mrb_value klass)
{
    mrb_value pointer, val;
    struct mrb_ffi_type *mft;
    mrb_get_args(mrb, "oo", &pointer, &val);

    mft = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(klass));
    mft->mrb_to_c(mrb, val, cfunc_pointer_ptr(pointer));

    return val;
}


mrb_value
cfunc_type_get_value(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    struct mrb_ffi_type *mft = rclass_to_mrb_ffi_type(mrb, mrb_class(mrb, self));
    return mft->data_to_mrb(mrb, data);
}


mrb_value
cfunc_type_set_value(mrb_state *mrb, mrb_value self)
{
    mrb_value val;
    struct cfunc_type_data *data;
    struct mrb_ffi_type *mft;
    mrb_get_args(mrb, "o", &val);
    
    data = (struct cfunc_type_data*)DATA_PTR(self);
    mft = rclass_to_mrb_ffi_type(mrb, mrb_class(mrb, self));
    mft->mrb_to_data(mrb, val, data);

    return val;
}


mrb_value
cfunc_type_addr(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);

    mrb_value ptr;
    if(data->refer) {
        ptr = cfunc_pointer_new_with_pointer(mrb, data->value._pointer, false);
    }
    else {
        ptr = cfunc_pointer_new_with_pointer(mrb, &data->value._pointer, false);
    }

    mrb_obj_iv_set(mrb, mrb_obj_ptr(ptr), mrb_intern_lit(mrb, "parent_pointer"), self); // keep for GC

    return ptr;
}


static
mrb_value int64_to_mrb(mrb_state *mrb, int64_t val)
{
    if(val < MRB_INT_MIN || val > MRB_INT_MAX) {
        return mrb_float_value(mrb, val);
    }
    else {
        return mrb_fixnum_value(val);
    }   
}

static
int64_t mrb_to_int64(mrb_state *mrb, mrb_value val)
{
    switch(mrb_type(val)) {
    case MRB_TT_FIXNUM:
        return mrb_fixnum(val);
     
    case MRB_TT_FLOAT:
        return mrb_float(val);

    case MRB_TT_FALSE:
        return 0;

    case MRB_TT_TRUE:
        return 1;

    case MRB_TT_DATA:
    case MRB_TT_OBJECT:
        {
            mrb_value result = mrb_funcall(mrb, val, "to_i", 0);
            switch(mrb_type(result)) {
            case MRB_TT_FIXNUM:
                return mrb_fixnum(result);
             
            case MRB_TT_FALSE:
                return 0;

            case MRB_TT_TRUE:
                return 1;
            default:
                return 0;  // can't reach here
            }
        }

    default:
        mrb_raisef(mrb, E_TYPE_ERROR, "type mismatch: %S given",
                   mrb_class_path(mrb, mrb_class(mrb, val)));
    }
    return 0; // can't reach here
}


static
mrb_float float_value(mrb_state *mrb, mrb_value val)
{
    switch(mrb_type(val)) {
    case MRB_TT_FIXNUM:
        return mrb_fixnum(val);
     
    case MRB_TT_FLOAT:
        return mrb_float(val);

    case MRB_TT_FALSE:
        return 0.0;

    case MRB_TT_TRUE:
        return 1.0;

    case MRB_TT_DATA:
    case MRB_TT_OBJECT:
        {
            mrb_value result = mrb_funcall(mrb, val, "to_f", 0);
            switch(mrb_type(result)) {
            case MRB_TT_FLOAT:
                return mrb_float(result);
             
            case MRB_TT_FALSE:
                return 0.0;

            case MRB_TT_TRUE:
                return 1.0;

            default:
                return 0.0; // can't reach here
            }
        }
        
    default:
        mrb_raisef(mrb, E_TYPE_ERROR, "type mismatch: %S given",
                   mrb_class_path(mrb, mrb_class(mrb, val)));
    }
    return 0.0; // can't reach here
}


// uint64 specific
mrb_value
cfunc_uint64_class_get(mrb_state *mrb, mrb_value klass)
{
    mrb_value pointer;
    uint64_t uint64;
    mrb_get_args(mrb, "o", &pointer);

    uint64 = *(uint64_t*)cfunc_pointer_ptr(pointer);

    if(uint64 > MRB_INT_MAX) {
        mrb_raise(mrb, E_TYPE_ERROR, "too big. Use low, high");
    }
    return int64_to_mrb(mrb, uint64);
}


mrb_value
cfunc_uint64_divide(mrb_state *mrb, mrb_value self)
{
    mrb_value mdivider;
    mrb_int divider;
    uint64_t result;
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    uint64_t uint64 = data->value._uint64;
    if(data->refer) {
        uint64 = *(uint64_t*)data->value._pointer;
    }
    
    mrb_get_args(mrb, "o", &mdivider);
    divider = mrb_fixnum(mdivider);    
    result = uint64 / divider;
    
    if(result > UINT32_MAX) {
        mrb_raise(mrb, E_TYPE_ERROR, "result too big.");
    }
    
    return int64_to_mrb(mrb, uint64 / divider );
}


mrb_value
cfunc_uint64_get_value(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    uint64_t uint64 = data->value._uint64;
    if(data->refer) {
        uint64 = *(uint64_t*)data->value._pointer;
    }
    if(uint64 > UINT32_MAX) {
        mrb_raise(mrb, E_TYPE_ERROR, "too big. Use low, high");
    }

    return int64_to_mrb(mrb, uint64);
}


mrb_value
cfunc_uint64_get_low(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    return int64_to_mrb(mrb, data->value._uint64 & 0xffffffff);
}


mrb_value
cfunc_uint64_set_low(mrb_state *mrb, mrb_value self)
{
    mrb_value val;
    struct cfunc_type_data *data;
    mrb_get_args(mrb, "o", &val);
    
    data = (struct cfunc_type_data*)DATA_PTR(self);
    data->value._uint64 = (data->value._uint64 & 0xffffffff00000000) | (((uint64_t)mrb_to_int64(mrb, val)) & 0xffffffff);
    return val;
}


mrb_value
cfunc_uint64_get_high(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    return int64_to_mrb(mrb, data->value._uint64 >> 32);
}


mrb_value
cfunc_uint64_set_high(mrb_state *mrb, mrb_value self)
{
    mrb_value val;
    struct cfunc_type_data *data;
    mrb_get_args(mrb, "o", &val);
    
    data = (struct cfunc_type_data*)DATA_PTR(self);
    data->value._uint64 = (data->value._uint64 & 0x00000000ffffffff) | (((uint64_t)mrb_to_int64(mrb, val)) << 32);
    
    return val;
}


mrb_value
cfunc_uint64_to_s(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    char str[65];
    snprintf(str, sizeof(str), "%" PRIu64, data->value._uint64);
    return mrb_str_new_cstr(mrb, str);
}




// sint64 specific
mrb_value
cfunc_sint64_class_get(mrb_state *mrb, mrb_value klass)
{
    mrb_value pointer;
    int64_t sint64;
    mrb_get_args(mrb, "o", &pointer);

    sint64 = *(int64_t*)cfunc_pointer_ptr(pointer);

    if(sint64 > MRB_INT_MAX || sint64 < MRB_INT_MIN) {
        mrb_raise(mrb, E_TYPE_ERROR, "out of range. Use low, high");
    }
    return int64_to_mrb(mrb, sint64);
}


mrb_value
cfunc_sint64_get_value(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    int64_t sint64 = data->value._sint64;
    if(data->refer) {
        sint64 = *(int64_t*)data->value._pointer;
    }
    if(sint64 > INT32_MAX || sint64 < INT32_MIN) {
        mrb_raise(mrb, E_TYPE_ERROR, "out of range. Use low, high");
    }
    return int64_to_mrb(mrb, sint64);
}


mrb_value
cfunc_int64_to_s(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);
    char str[65];
    snprintf(str, sizeof(str), "%" PRId64, data->value._sint64);
    return mrb_str_new_cstr(mrb, str);
}


// nil specific
mrb_value
cfunc_nil_addr(mrb_state *mrb, mrb_value self)
{
    struct cfunc_type_data *data = (struct cfunc_type_data*)DATA_PTR(self);

    mrb_value ptr;
    if(data->refer) {
        ptr = cfunc_pointer_new_with_pointer(mrb, data->value._pointer, false);
    }
    else {
        ptr = cfunc_pointer_new_with_pointer(mrb, &data->value._pointer, false);
    }

    mrb_obj_iv_set(mrb, mrb_obj_ptr(ptr), mrb_intern_lit(mrb, "parent_pointer"), self); // keep for GC

    return ptr;
}

static mrb_value
cfunc_nil_align(mrb_state *mrb, mrb_value klass)
{
    return mrb_fixnum_value(ffi_type_pointer.alignment);
}


static mrb_value
cfunc_nil_size(mrb_state *mrb, mrb_value klass)
{
    return mrb_fixnum_value(ffi_type_pointer.size);
}


// void specific
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


// macros
#define define_cfunc_type_(name, ffi_type_ptr, ctype, c_to_mrb, mrb_to_c) \
\
static mrb_value \
cfunc_type_ffi_##name##_c_to_mrb(mrb_state *mrb, void *p) \
{ \
    return c_to_mrb(mrb, *(ctype*)p); \
} \
\
static void \
cfunc_type_ffi_##name##_mrb_to_c(mrb_state *mrb, mrb_value val, void *p) \
{ \
    *(ctype*)p = mrb_to_c(mrb, val); \
} \
\
static mrb_value \
cfunc_type_ffi_##name##_data_to_mrb(mrb_state *mrb, struct cfunc_type_data *data) \
{ \
    if(data->refer) { \
        return c_to_mrb(mrb, *(ctype*)data->value._pointer); \
    } \
    else { \
        return c_to_mrb(mrb, data->value._##name); \
    } \
} \
\
static void \
cfunc_type_ffi_##name##_mrb_to_data(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data) \
{ \
    if(data->refer) { \
        *(ctype*)data->value._pointer = mrb_to_c(mrb, val); \
    } \
    else { \
        data->value._##name = mrb_to_c(mrb, val); \
    } \
}

// macros
#define define_cfunc_type(name, ffi_type_ptr, ctype, c_to_mrb, mrb_to_c) \
\
static mrb_value \
cfunc_type_ffi_##name##_c_to_mrb(mrb_state *mrb, void *p) \
{ \
    return c_to_mrb(mrb, *(ctype*)p); \
} \
\
static void \
cfunc_type_ffi_##name##_mrb_to_c(mrb_state *mrb, mrb_value val, void *p) \
{ \
    *(ctype*)p = mrb_to_c(mrb, val); \
} \
\
static mrb_value \
cfunc_type_ffi_##name##_data_to_mrb(mrb_state *mrb, struct cfunc_type_data *data) \
{ \
    if(data->refer) { \
        return c_to_mrb(mrb, *(ctype*)data->value._pointer); \
    } \
    else { \
        return c_to_mrb(mrb, data->value._##name); \
    } \
} \
\
static void \
cfunc_type_ffi_##name##_mrb_to_data(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data) \
{ \
    if(data->refer) { \
        *(ctype*)data->value._pointer = mrb_to_c(mrb, val); \
    } \
    else { \
        data->value._##name = mrb_to_c(mrb, val); \
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

define_cfunc_type(sint8, &ffi_type_sint8, int8_t, int64_to_mrb, mrb_to_int64)
define_cfunc_type(uint8, &ffi_type_uint8, uint8_t, int64_to_mrb, mrb_to_int64)

define_cfunc_type(sint16, &ffi_type_sint16, int16_t, int64_to_mrb, mrb_to_int64)
define_cfunc_type(uint16, &ffi_type_uint16, uint16_t, int64_to_mrb, mrb_to_int64)

define_cfunc_type(sint32, &ffi_type_sint32, int32_t, int64_to_mrb, mrb_to_int64)
define_cfunc_type(uint32, &ffi_type_uint32, uint32_t, int64_to_mrb, mrb_to_int64)

define_cfunc_type(sint64, &ffi_type_sint64, int64_t, int64_to_mrb, mrb_to_int64)
define_cfunc_type(uint64, &ffi_type_uint64, uint64_t, int64_to_mrb, mrb_to_int64)

define_cfunc_type(float, &ffi_type_float, float, mrb_float_value, float_value)
define_cfunc_type(double, &ffi_type_double, double, mrb_float_value, float_value)


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
    struct cfunc_state *state = cfunc_state(mrb, module);
    struct RClass *type_class = mrb_define_class_under(mrb, module, "Type", mrb->object_class);
    int ai, map_size, i;
    mrb_value mod;
    struct RClass *uint64_class, *sint64_class;
    MRB_SET_INSTANCE_TT(type_class, MRB_TT_DATA);
    state->type_class = type_class;
    set_cfunc_state(mrb, type_class, state);

    ai = mrb_gc_arena_save(mrb);
    mrb_define_class_method(mrb, type_class, "refer", cfunc_type_class_refer, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, type_class, "size", cfunc_type_size, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, type_class, "align", cfunc_type_align, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, type_class, "get", cfunc_type_class_get, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, type_class, "set", cfunc_type_class_set, MRB_ARGS_REQ(2));

    mrb_define_method(mrb, type_class, "initialize", cfunc_type_initialize, MRB_ARGS_ANY());
    mrb_define_method(mrb, type_class, "value", cfunc_type_get_value, MRB_ARGS_NONE());
    mrb_define_method(mrb, type_class, "value=", cfunc_type_set_value, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, type_class, "addr", cfunc_type_addr, MRB_ARGS_NONE());
    mrb_define_method(mrb, type_class, "to_ffi_value", cfunc_type_addr, MRB_ARGS_NONE());
    DONE;

    map_size = sizeof(types) / sizeof(struct mrb_ffi_type);
    for(i = 0; i < map_size; ++i) {
        struct RClass *new_class = mrb_define_class_under(mrb, module, types[i].name, type_class);
        mrb_value ffi_type = mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &cfunc_class_ffi_data_type, &types[i]));
        mrb_obj_iv_set(mrb, (struct RObject*)new_class, mrb_intern_lit(mrb, "@ffi_type"), ffi_type);
    }
    DONE;
    
    mod = mrb_obj_value(module);
    state->void_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "Void")));
    state->uint8_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "UInt8")));
    state->sint8_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "SInt8")));
    state->uint16_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "UInt16")));
    state->sint16_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "SInt16")));
    state->uint32_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "UInt32")));
    state->sint32_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "SInt32")));
    state->uint64_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "UInt64")));
    state->sint64_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "SInt64")));
    state->float_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "Float")));
    state->double_class = mrb_class_ptr(mrb_const_get(mrb, mod, mrb_intern_lit(mrb, "Double")));
    DONE;

    mrb_define_class_method(mrb, mrb->nil_class, "size", cfunc_nil_size, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, mrb->nil_class, "align", cfunc_nil_align, MRB_ARGS_NONE());
    DONE;

    // uint64 specific
    uint64_class = state->uint64_class;
    mrb_define_class_method(mrb, uint64_class, "get", cfunc_uint64_class_get, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, uint64_class, "value", cfunc_uint64_get_value, MRB_ARGS_NONE());
    mrb_define_method(mrb, uint64_class, "low", cfunc_uint64_get_low, MRB_ARGS_NONE());
    mrb_define_method(mrb, uint64_class, "low=", cfunc_uint64_set_low, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, uint64_class, "high", cfunc_uint64_get_high, MRB_ARGS_NONE());
    mrb_define_method(mrb, uint64_class, "high=", cfunc_uint64_set_high, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, uint64_class, "to_s", cfunc_uint64_to_s, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, uint64_class, "divide", cfunc_uint64_divide, MRB_ARGS_REQ(1));
    DONE;
    
    // sint64 specific
    sint64_class = state->sint64_class;
    mrb_define_class_method(mrb, sint64_class, "get", cfunc_sint64_class_get, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, sint64_class, "value", cfunc_sint64_get_value, MRB_ARGS_NONE());
    mrb_define_method(mrb, sint64_class, "low", cfunc_uint64_get_low, MRB_ARGS_NONE());
    mrb_define_method(mrb, sint64_class, "low=", cfunc_uint64_set_low, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, sint64_class, "high", cfunc_uint64_get_high, MRB_ARGS_NONE());
    mrb_define_method(mrb, sint64_class, "high=", cfunc_uint64_set_high, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, sint64_class, "to_s", cfunc_int64_to_s, MRB_ARGS_REQ(1));
    DONE;
}
