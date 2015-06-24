//
//  CFunc initializer method in C
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc.h"

#include "cfunc_call.h"
#include "cfunc_pointer.h"
#include "cfunc_struct.h"
#include "cfunc_closure.h"
#include "cfunc_type.h"
#include "cfunc_rubyvm.h"
#include "cfunc_platform.h"

#include "mruby/proc.h"
#include "mruby/dump.h"

#include <setjmp.h>
#include <errno.h>

mrb_value
cfunc_mrb_state(mrb_state *mrb, mrb_value klass)
{
    return cfunc_pointer_new_with_pointer(mrb, mrb, false);
}

mrb_value
cfunc_errno(mrb_state *mrb, mrb_value klass)
{
    return mrb_fixnum_value(errno);
}

mrb_value
cfunc_strerror(mrb_state *mrb, mrb_value klass)
{
    mrb_value msg;
    msg = mrb_str_new_cstr(mrb, strerror(errno));
    return msg;
}


void
mrb_mruby_cfunc_gem_init(mrb_state* mrb)
{
    struct RClass *ns = mrb_define_module(mrb, "CFunc");
    struct cfunc_state *state = mrb_malloc(mrb, sizeof(struct cfunc_state));
    set_cfunc_state(mrb, ns, state);
    state->namespace = ns;

	int ai = mrb_gc_arena_save(mrb);
    init_cfunc_type(mrb, ns); mrb_gc_arena_restore(mrb, ai);
    init_cfunc_pointer(mrb, ns); mrb_gc_arena_restore(mrb, ai);
    init_cfunc_struct(mrb, ns); mrb_gc_arena_restore(mrb, ai);
    init_cfunc_closure(mrb, ns); mrb_gc_arena_restore(mrb, ai);
    init_cfunc_call(mrb, ns); mrb_gc_arena_restore(mrb, ai);
    init_cfunc_rubyvm(mrb, ns); mrb_gc_arena_restore(mrb, ai);
	init_cfunc_platform(mrb, ns); mrb_gc_arena_restore(mrb, ai);

    mrb_define_class_method(mrb, ns, "mrb_state", cfunc_mrb_state, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, ns, "errno", cfunc_errno, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, ns, "strerror", cfunc_strerror, MRB_ARGS_NONE());
}

void
mrb_mruby_cfunc_gem_final(mrb_state* mrb)
{
    mrb_free(mrb, cfunc_state(mrb, NULL));
}
