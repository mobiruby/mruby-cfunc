//
//  CFunc::Pointer class
// 
//  See Copyright Notice in cfunc.h
//

#ifndef cfunc_pointer_h
#define cfunc_pointer_h

#include "cfunc.h"
#include "cfunc_type.h"
#include "mruby/data.h"
#include <stdbool.h>

#define cfunc_pointer_ptr(v) ( \
    ((struct cfunc_type_data*)DATA_PTR(v))->refer \
    ? *(void**)(((struct cfunc_type_data*)DATA_PTR(v))->value._pointer) \
    : ((struct cfunc_type_data*)DATA_PTR(v))->value._pointer \
)

struct RClass* cfunc_pointer_class(mrb_state *mrb);

void* get_cfunc_pointer_data(struct cfunc_type_data *data);
void set_cfunc_pointer_data(struct cfunc_type_data *data, void *p);

void init_cfunc_pointer(mrb_state *mrb, struct RClass* module);

mrb_value cfunc_pointer_new_with_pointer(mrb_state *mrb, void *p, bool autorelease);


#endif
