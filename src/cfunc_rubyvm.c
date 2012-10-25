//
//  CFunc::RubyVM class
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc_rubyvm.h"
#include "cfunc_type.h"

#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/dump.h"
#include "mruby/proc.h"
#include "mruby/compile.h"

#include "ffi.h"

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <dlfcn.h>

struct task_arg {
    enum mrb_vtype tt;
    union {
        mrb_float f;
        mrb_int i;
        struct {
            char* ptr;
            int len;
        } string;
        struct {
            struct task_arg **ptr;
            int len;
        } array;
    } value;
};

struct queue_task {
    char* name;
    struct task_arg **args;
    int args_len;
};


struct task_arg* mrb_value_to_task_arg(mrb_state *mrb, mrb_value v)
{
    struct task_arg *arg = malloc(sizeof(struct task_arg));

    arg->tt = mrb_type(v);
    switch (mrb_type(v)) {
    case MRB_TT_FALSE:
    case MRB_TT_TRUE:
    case MRB_TT_FIXNUM:
        arg->value.i = v.value.i;
        break;

    case MRB_TT_FLOAT:
        arg->value.f = v.value.f;
        break;

    case MRB_TT_SYMBOL:
        {
            const char* name = mrb_sym2name_len(mrb, v.value.sym, &arg->value.string.len);
            arg->value.string.ptr = malloc(arg->value.string.len+1);
            memcpy(arg->value.string.ptr, name, arg->value.string.len+1);
        }
        break;

    case MRB_TT_STRING:
        {
            struct RString *str = mrb_str_ptr(v);
            arg->value.string.len = str->len;
            arg->value.string.ptr = malloc(arg->value.string.len+1);
            memcpy(arg->value.string.ptr, str->ptr, arg->value.string.len+1);
        }
        break;

    case MRB_TT_ARRAY:
        {
            struct RArray *ary = mrb_ary_ptr(v);

            arg->value.array.len = ary->len;
            arg->value.array.ptr = malloc(ary->len * sizeof(struct task_arg));

            for(int i=0; i<ary->len; i++) {
                arg->value.array.ptr[i] = mrb_value_to_task_arg(mrb, ary->ptr[i]);
            }
        }
        break;

    default:
        mrb_raise(mrb, E_TYPE_ERROR, "cannot pass to other RubyVM");
        break;
    }

    return arg;
}


mrb_value task_arg_to_mrb_value(mrb_state *mrb, struct task_arg* arg)
{
    mrb_value v;

    mrb_type(v) = arg->tt;
    switch (arg->tt) {
    case MRB_TT_FALSE:
    case MRB_TT_TRUE:
    case MRB_TT_FIXNUM:
        v.value.i = arg->value.i;
        break;

    case MRB_TT_FLOAT:
        v.value.f = arg->value.f;
        break;

    case MRB_TT_SYMBOL:
        v.value.sym = mrb_intern(mrb, arg->value.string.ptr);
        break;

    case MRB_TT_STRING:
        v = mrb_str_new(mrb, arg->value.string.ptr, arg->value.string.len);
        break;

    case MRB_TT_ARRAY:
        {
            v = mrb_ary_new_capa(mrb, arg->value.array.len);
            struct RArray *ary = mrb_ary_ptr(v);
            ary->len = arg->value.array.len;
            for(int i=0; i<arg->value.array.len; i++) {
                ary->ptr[i] = task_arg_to_mrb_value(mrb, arg->value.array.ptr[i]);
            }
        }
        break;

    default:
        mrb_raise(mrb, E_TYPE_ERROR, "cannot pass to other RubyVM");
        break;
    }

    return v;
}

void
free_task_arg(struct task_arg* arg)
{
    switch (arg->tt) {
    case MRB_TT_SYMBOL:
    case MRB_TT_STRING:
        free(arg->value.string.ptr);
        break;

    case MRB_TT_ARRAY:
        {
            for(int i=0; i<arg->value.array.len; i++) {
                free_task_arg(arg->value.array.ptr[i]);
            }
            free(arg->value.array.ptr);
        }
        break;

    default:
        break;
    }

}


static void
cfunc_rubyvm_data_destructor(mrb_state *mrb, void *p_)
{
    // todo
};


const struct mrb_data_type cfunc_rubyvm_data_type = {
    "cfunc_rubyvm", cfunc_rubyvm_data_destructor,
};

void*
cfunc_rubyvm_open(void *args)
{
    struct cfunc_rubyvm_data *data = args;
    mrb_state *mrb = mrb_open();
    data->state = mrb;
    int n = mrb_read_irep(mrb, data->mrb_data);

    mrb_run(mrb, mrb_proc_new(mrb, mrb->irep[n]), mrb_top_self(mrb));

    if (mrb->exc) {
        return NULL;
    }

    while(true) {
        pthread_mutex_lock(&data->mutex);
        while(data->queue->length == 0) {
            pthread_cond_wait(&data->cond, &data->mutex);
        }
        
        struct queue_task *task = vector_dequeue(data->queue);
        mrb_sym taskname = mrb_intern(mrb, task->name);

        int args_len = task->args_len;
        mrb_value *args = malloc(sizeof(struct task_arg) * task->args_len);
        for(int i=0; i<task->args_len; ++i) {
            args[i] = task_arg_to_mrb_value(data->state, task->args[i]);
            free_task_arg(task->args[i]);
        }
        free(task->args);
        free(task->name);
        free(task);

        pthread_mutex_unlock(&data->mutex);

        mrb_funcall_argv(mrb, mrb_top_self(data->state), taskname, args_len, args);
        free(args);
    }

    return NULL;
}


mrb_value
cfunc_rubyvm_dispatch_async(mrb_state *mrb, mrb_value self)
{
    struct cfunc_rubyvm_data *data = mrb_get_datatype(mrb, self, &cfunc_rubyvm_data_type);

    mrb_value name_obj, *args;
    int args_len;
    mrb_get_args(mrb, "o*", &name_obj, &args, &args_len);

    struct queue_task *task = malloc(sizeof(struct queue_task));

    const char* name = mrb_string_value_ptr(mrb, name_obj);
    int name_len = strlen(name);
    task->name = malloc(name_len+1);
    strncpy(task->name, name, name_len+1);

    task->args_len = args_len;
    task->args = malloc(sizeof(struct task_arg) * task->args_len);
    for(int i=0; i<args_len; ++i) {
        task->args[i] = mrb_value_to_task_arg(mrb, args[i]);
    }

    pthread_mutex_lock(&data->mutex);
    vector_enqueue(data->queue, task);
    pthread_cond_signal(&data->cond);
    pthread_mutex_unlock(&data->mutex);

    return mrb_nil_value();
}


mrb_value
cfunc_rubyvm_class_thread(mrb_state *mrb, mrb_value klass)
{
    // init bindle data with RubyVM object
    struct RClass *c = mrb_class_ptr(klass);
    struct cfunc_rubyvm_data *data = malloc(sizeof(struct cfunc_rubyvm_data));
    mrb_value self = mrb_obj_value((struct RObject *)Data_Wrap_Struct(mrb, c, &cfunc_rubyvm_data_type, data));

    // load script
    mrb_value filename, str;
    mrb_get_args(mrb, "S", &filename);
    str = mrb_str_new_cstr(mrb, "mruby_data_");
    mrb_str_concat(mrb, str, mrb_str_new(mrb, RSTRING_PTR(filename), RSTRING_LEN(filename)));

    void *dlh = dlopen(NULL, RTLD_LAZY);
    data->mrb_data = (const char *)dlsym(dlh, RSTRING_PTR(str));

    if (!data->mrb_data) {
        dlclose(dlh);
        mrb_raisef(mrb, E_SCRIPT_ERROR, "file '%s' not found.", RSTRING_PTR(str));
    }

    // initial pthread
    data->queue = create_vector();
    pthread_mutex_init(&data->mutex, NULL);
    pthread_cond_init(&data->cond, NULL);
    pthread_create(&data->thread, NULL, cfunc_rubyvm_open, (void*)data);

    return self;
}


void
init_cfunc_rubyvm(mrb_state *mrb, struct RClass* module)
{
    struct RClass *rubyvm_class = mrb_define_class_under(mrb, module, "RubyVM", mrb->object_class);
    cfunc_state(mrb)->rubyvm_class = rubyvm_class;
    
    mrb_define_class_method(mrb, rubyvm_class, "thread", cfunc_rubyvm_class_thread, ARGS_REQ(1));
    mrb_define_method(mrb, rubyvm_class, "dispatch_async", cfunc_rubyvm_dispatch_async, ARGS_ANY());
}
