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


mrb_value
cfunc_mrb_state(mrb_state *mrb, mrb_value klass)
{
    return cfunc_pointer_new_with_pointer(mrb, mrb, false);
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

    mrb_define_class_method(mrb, ns, "mrb_state", cfunc_mrb_state, ARGS_NONE());
}

void
mrb_mruby_cfunc_gem_final(mrb_state* mrb)
{
    mrb_free(mrb, cfunc_state(mrb, NULL));
}
