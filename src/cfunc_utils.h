//
//  mruby utilities for CFunc
// 
//  See Copyright Notice in cfunc.h
//

#ifndef cfunc_mrb_utils_h
#define cfunc_mrb_utils_h

#include "mruby.h"

void
cfunc_mrb_raise_without_jump(mrb_state *mrb, struct RClass *c, const char *fmt, ...);

#endif
