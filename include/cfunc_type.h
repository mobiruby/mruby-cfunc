//
//  CFunc Basic types
// 
//  See Copyright Notice in cfunc.h
//


#ifndef cfunc_type_h
#define cfunc_type_h

#include "cfunc.h"
#include "ffi.h"


#define MOBI_C_TYPE_HEADER \
    union { \
        void *_pointer; \
        int8_t _sint8; \
        uint8_t _uint8; \
        int16_t _sint16; \
        uint16_t _uint16; \
        int32_t _sint32; \
        uint32_t _uint32; \
        int64_t _sint64; \
        uint64_t _uint64; \
        double _double; \
        float _float; \
    } value; \
    uint8_t refer: 1; \
    uint8_t autofree: 1;

struct cfunc_type_data {
    MOBI_C_TYPE_HEADER
};

struct mrb_ffi_type {
    const char* name;
    ffi_type* ffi_type_value;
    mrb_value (*data_to_mrb)(mrb_state *mrb, struct cfunc_type_data *data);
    void (*mrb_to_data)(mrb_state *mrb, mrb_value val, struct cfunc_type_data *data);
    mrb_value (*c_to_mrb)(mrb_state *mrb, void *p);
    void (*mrb_to_c)(mrb_state *mrb, mrb_value val, void *p);
};

struct mrb_ffi_type* rclass_to_mrb_ffi_type(mrb_state *mrb, struct RClass *cls);
struct mrb_ffi_type* mrb_value_to_mrb_ffi_type(mrb_state *mrb, mrb_value value);
mrb_value cfunc_type_class(mrb_state *mrb, const char* name);

void init_cfunc_type(mrb_state *mrb, struct RClass* module);

#endif
