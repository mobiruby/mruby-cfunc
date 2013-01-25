/*
** cfunc-mruby - Interface to C functions on mruby
**
** Copyright (c) MobiRuby developers 2012-
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
** [ MIT license: http://www.opensource.org/licenses/mit-license.php ]
*/

#ifndef cfunc_h
#define cfunc_h

#include "mruby.h"
#include "mruby/variable.h"
#include "mruby/value.h"

struct cfunc_state {
    struct RClass *namespace;

    struct RClass *type_class;
    struct RClass *void_class;
    struct RClass *uint8_class;
    struct RClass *sint8_class;
    struct RClass *uint16_class;
    struct RClass *sint16_class;
    struct RClass *uint32_class;
    struct RClass *sint32_class;
    struct RClass *uint64_class;
    struct RClass *sint64_class;
    struct RClass *float_class;
    struct RClass *double_class;
    struct RClass *pointer_class;
    struct RClass *struct_class;
    struct RClass *closure_class;
    struct RClass *rubyvm_class;
    struct RClass *rubyvm_task_class;
};

static inline struct cfunc_state *
cfunc_state(mrb_state *mrb, struct RClass* obj)
{
    if(obj == NULL) {
        obj = (struct RClass*) mrb_object(mrb_vm_const_get(mrb, mrb_intern(mrb, "CFunc")));
    }
    mrb_value state = mrb_mod_cv_get(mrb, obj, mrb_intern(mrb, "cfunc_state"));
    return (struct cfunc_state *)mrb_voidp(state);
}


static inline void
set_cfunc_state(mrb_state *mrb, struct RClass* klass, struct cfunc_state *state)
{
    mrb_value mstate = mrb_voidp_value(state);
    mrb_mod_cv_set(mrb, klass, mrb_intern(mrb, "cfunc_state"), mstate);
}

#endif
