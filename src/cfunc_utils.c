//
//  mruby utilities for CFunc
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc_utils.h"
#include "mruby/class.h"
#include "mruby/variable.h"

#include <stdarg.h>
#include <stdio.h>


void
cfunc_mrb_raise_without_jump(mrb_state *mrb, struct RClass *c, const char *fmt, ...)
{
    va_list args;
    char buf[256];
    int n;

    va_start(args, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n < 0) {
      n = 0;
    }
    mrb->exc = (struct RObject*)mrb_object(mrb_exc_new(mrb, c, buf, n));
}
