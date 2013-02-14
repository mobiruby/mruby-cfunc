//
//  CFunc::call method
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc_platform.h"
#include "mruby/string.h"
#include "mruby/class.h"

static mrb_value
cfunc_platform_is_posix(mrb_state *mrb, mrb_value self)
{
#ifdef __USE_POSIX
    return mrb_true_value();
#else
    return mrb_false_value();
#endif
}


static mrb_value
cfunc_platform_is_win32(mrb_state *mrb, mrb_value self)
{
#ifdef _WIN32
    return mrb_true_value();
#else
    return mrb_false_value();
#endif
}


static mrb_value
cfunc_platform_is_darwin(mrb_state *mrb, mrb_value self)
{
#ifdef __APPLE__
    return mrb_true_value();
#else
    return mrb_false_value();
#endif
}


void init_cfunc_platform(mrb_state *mrb, struct RClass* module)
{
    struct RClass *struct_class = mrb_define_class_under(mrb, module, "Platform", mrb->object_class);
    
    mrb_define_class_method(mrb, struct_class, "is_posix?", cfunc_platform_is_posix, ARGS_NONE());
    mrb_define_class_method(mrb, struct_class, "is_win32?", cfunc_platform_is_win32, ARGS_NONE());
    mrb_define_class_method(mrb, struct_class, "is_darwin?", cfunc_platform_is_darwin, ARGS_NONE());
}
