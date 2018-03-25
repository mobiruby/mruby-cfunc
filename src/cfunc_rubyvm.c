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
#include <signal.h>
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
    enum queue_task_status {
        queue_task_queued,
        queue_task_running,
        queue_task_finished
    } status;

    struct task_arg **args;
    int args_len;

    struct task_arg *result;

    bool sync;
    pthread_mutex_t sync_mutex;
    pthread_cond_t sync_cond;

    int refcount;
};


struct task_arg* mrb_value_to_task_arg(mrb_state *mrb, mrb_value v)
{
    struct task_arg *arg = mrb_malloc(mrb, sizeof(struct task_arg));

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
            mrb_int len;
            const char* name = mrb_sym2name_len(mrb, v.value.sym, &len);
            arg->value.string.len = len;
            arg->value.string.ptr = mrb_malloc(mrb, len + 1);
            memcpy(arg->value.string.ptr, name, len + 1);
        }
        break;

    case MRB_TT_STRING:
        {
            arg->value.string.len = RSTRING_LEN(v);
            arg->value.string.ptr = mrb_malloc(mrb, arg->value.string.len+1);
            memcpy(arg->value.string.ptr, RSTRING_PTR(v), arg->value.string.len+1);
        }
        break;

    case MRB_TT_ARRAY:
        {
            int i;
            arg->value.array.len = RARRAY_LEN(v);
            arg->value.array.ptr = mrb_malloc(mrb, RARRAY_LEN(v) * sizeof(struct task_arg));

            for(i=0; i<RARRAY_LEN(v); i++) {
                arg->value.array.ptr[i] = mrb_value_to_task_arg(mrb, RARRAY_PTR(v)[i]);
            }
        }
        break;

    default:
        mrb_free(mrb, arg);
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
        v.value.sym = mrb_intern_cstr(mrb, arg->value.string.ptr);
        break;

    case MRB_TT_STRING:
        v = mrb_str_new(mrb, arg->value.string.ptr, arg->value.string.len);
        break;

    case MRB_TT_ARRAY:
        {
            int i;
            v = mrb_ary_new_capa(mrb, arg->value.array.len);
            mrb_ary_resize(mrb, v, arg->value.array.len);
            for(i=0; i<arg->value.array.len; i++) {
                RARRAY_PTR(v)[i] = task_arg_to_mrb_value(mrb, arg->value.array.ptr[i]);
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
free_task_arg(mrb_state *mrb, struct task_arg* arg)
{
    if(!arg) {
        return;
    }
    switch (arg->tt) {
    case MRB_TT_SYMBOL:
    case MRB_TT_STRING:
        mrb_free(mrb, arg->value.string.ptr);
        break;

    case MRB_TT_ARRAY:
        {
            int i;
            for(i=0; i<arg->value.array.len; i++) {
                free_task_arg(mrb, arg->value.array.ptr[i]);
                mrb_free(mrb, arg->value.array.ptr[i]);
            }
            mrb_free(mrb, arg->value.array.ptr);
        }
        break;

    default:
        break;
    }
}


void
free_queue_task(mrb_state *mrb, struct queue_task* task)
{
    task->refcount--;
    if(task->refcount < 1) {
        int i;
        for(i=0; i<task->args_len; ++i) {
            free_task_arg(mrb, task->args[i]);
            mrb_free(mrb, task->args[i]);
        }
        mrb_free(mrb, task->args);

        free_task_arg(mrb, task->result);
        mrb_free(mrb, task->result);

        mrb_free(mrb, task->name);
        mrb_free(mrb, task);
    }
}



static void
cfunc_rubyvm_data_destructor(mrb_state *mrb, void *p)
{
    struct cfunc_rubyvm_data *vm = (struct cfunc_rubyvm_data*)p;
    pthread_cancel(vm->thread);

    while (vm->queue->length != 0) {
        struct queue_task *task = vector_dequeue(vm->queue);
        free_queue_task(mrb, task);
    }
    destroy_vector(vm->queue);

    mrb_close(vm->state);

    pthread_mutex_destroy(&vm->queue_mutex);
    pthread_cond_destroy(&vm->queue_cond);
    mrb_free(mrb, vm);
}


const struct mrb_data_type cfunc_rubyvm_data_type = {
    "cfunc_rubyvm", cfunc_rubyvm_data_destructor,
};




static void
cfunc_rubyvm_task_data_destructor(mrb_state *mrb, void *p)
{
    free_queue_task(mrb, (struct queue_task*)p);
}


const struct mrb_data_type cfunc_rubyvm_task_data_type = {
    "cfunc_rubyvm_task", cfunc_rubyvm_task_data_destructor,
};


void*
cfunc_rubyvm_open(void *args)
{
    struct cfunc_rubyvm_data *data = args;
    mrb_state *mrb = mrb_open();
    mrb_irep* irep;
    data->state = mrb;
    
#ifdef DISABLE_GEMS
    init_cfunc_module(mrb);
#endif

    irep = mrb_read_irep(mrb, data->mrb_data);

    mrb_run(mrb, mrb_proc_new(mrb, irep), mrb_top_self(mrb));

    mrb_irep_decref(mrb, irep);

    if (mrb->exc) {
        return NULL;
    }

    while(true) {
        struct queue_task *task;
        mrb_sym taskname;
        int args_len;
        mrb_value *args;
        int i;
        mrb_value result;
        pthread_mutex_lock(&data->queue_mutex);

        while(data->queue->length == 0) {
            pthread_cond_wait(&data->queue_cond, &data->queue_mutex);
        }
        
        task = vector_dequeue(data->queue);
        task->status = queue_task_running;
        taskname = mrb_intern_cstr(mrb, task->name);

        args_len = task->args_len;
        args = mrb_malloc(mrb, sizeof(struct task_arg) * task->args_len);
        for(i=0; i<task->args_len; ++i) {
            args[i] = task_arg_to_mrb_value(data->state, task->args[i]);
        }

        pthread_mutex_unlock(&data->queue_mutex);

        result = mrb_funcall_argv(mrb, mrb_top_self(data->state), taskname, args_len, args);
        task->result = mrb_value_to_task_arg(mrb, result);
        task->status = queue_task_finished;
        pthread_cond_signal(&task->sync_cond);

        mrb_free(mrb, args);
        free_queue_task(mrb, task);
    }

    return NULL;
}


mrb_value
cfunc_rubyvm_dispatch(mrb_state *mrb, mrb_value self)
{
    struct cfunc_rubyvm_data *data = mrb_data_check_get_ptr(mrb, self, &cfunc_rubyvm_data_type);

    mrb_value name_obj, *args;
    mrb_int args_len;
    struct queue_task *task;
    const char* name;
    int name_len;
    int i;
    struct cfunc_state *state;
    mrb_get_args(mrb, "o*", &name_obj, &args, &args_len);

    task = mrb_malloc(mrb, sizeof(struct queue_task));
    task->refcount = 2;
    task->result = NULL;
    task->status = queue_task_queued;

    pthread_mutex_init(&task->sync_mutex, NULL);
    pthread_cond_init(&task->sync_cond, NULL);

    name = mrb_str_to_cstr(mrb, mrb_str_to_str(mrb, name_obj));
    name_len = strlen(name);
    task->name = mrb_malloc(mrb, name_len+1);
    strncpy(task->name, name, name_len+1);

    task->args_len = args_len;
    task->args = mrb_malloc(mrb, sizeof(struct task_arg) * task->args_len);
    for(i=0; i<args_len; ++i) {
        task->args[i] = mrb_value_to_task_arg(mrb, args[i]);
    }

    pthread_mutex_lock(&data->queue_mutex);
    vector_enqueue(data->queue, task);
    pthread_cond_signal(&data->queue_cond);
    pthread_mutex_unlock(&data->queue_mutex);

    state = cfunc_state(mrb, mrb_obj_ptr(self)->c);
    return mrb_obj_value((struct RObject *)Data_Wrap_Struct(mrb, state->rubyvm_task_class, &cfunc_rubyvm_task_data_type, task));
}


mrb_value
cfunc_rubyvm_task_status(mrb_state *mrb, mrb_value self)
{
    struct queue_task *task = DATA_PTR(self);
    return mrb_fixnum_value(task->status);
}


mrb_value
cfunc_rubyvm_task_result(mrb_state *mrb, mrb_value self)
{
    struct queue_task *task = DATA_PTR(self);
    return task_arg_to_mrb_value(mrb, task->result);
}


mrb_value
cfunc_rubyvm_task_wait(mrb_state *mrb, mrb_value self)
{
    struct queue_task *task = DATA_PTR(self);
    if(task->status == queue_task_queued || task->status == queue_task_running) {
        pthread_mutex_lock(&task->sync_mutex);
        pthread_cond_wait(&task->sync_cond, &task->sync_mutex);
        pthread_mutex_unlock(&task->sync_mutex);
    }

    return task_arg_to_mrb_value(mrb, task->result);
}


mrb_value
cfunc_rubyvm_class_thread(mrb_state *mrb, mrb_value klass)
{
    // init bindle data with RubyVM object
    struct RClass *c = mrb_class_ptr(klass);
    struct cfunc_rubyvm_data *data = mrb_malloc(mrb, sizeof(struct cfunc_rubyvm_data));
    mrb_value self = mrb_obj_value((struct RObject *)Data_Wrap_Struct(mrb, c, &cfunc_rubyvm_data_type, data));
    void *dlh;

    // load script
    mrb_value filename, str;
    mrb_get_args(mrb, "S", &filename);
    str = mrb_str_new_lit(mrb, "mruby_data_");
    mrb_str_concat(mrb, str, mrb_str_new(mrb, RSTRING_PTR(filename), RSTRING_LEN(filename)));

    dlh = dlopen(NULL, RTLD_LAZY);
    data->mrb_data = (const uint8_t *)dlsym(dlh, RSTRING_PTR(str));

    if (!data->mrb_data) {
        dlclose(dlh);
        mrb_raisef(mrb, E_SCRIPT_ERROR, "file '%S' not found.", str);
    }

    // initial pthread
    data->queue = create_vector();
    pthread_mutex_init(&data->queue_mutex, NULL);
    pthread_cond_init(&data->queue_cond, NULL);
    pthread_create(&data->thread, NULL, cfunc_rubyvm_open, (void*)data);

    return self;
}


void
init_cfunc_rubyvm(mrb_state *mrb, struct RClass* module)
{
    struct cfunc_state *state = cfunc_state(mrb, module);
    struct RClass *rubyvm_task_class;

    struct RClass *rubyvm_class = mrb_define_class_under(mrb, module, "RubyVM", mrb->object_class);
    state->rubyvm_class = rubyvm_class;
    set_cfunc_state(mrb, rubyvm_class, state);

    mrb_define_class_method(mrb, rubyvm_class, "thread", cfunc_rubyvm_class_thread, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, rubyvm_class, "dispatch", cfunc_rubyvm_dispatch, MRB_ARGS_ANY());

    rubyvm_task_class = mrb_define_class_under(mrb, rubyvm_class, "Task", mrb->object_class);
    state->rubyvm_task_class = rubyvm_task_class;

    mrb_define_method(mrb, rubyvm_task_class, "wait", cfunc_rubyvm_task_wait, MRB_ARGS_NONE());
    mrb_define_method(mrb, rubyvm_task_class, "result", cfunc_rubyvm_task_result, MRB_ARGS_NONE());
    mrb_define_method(mrb, rubyvm_task_class, "status", cfunc_rubyvm_task_status, MRB_ARGS_NONE());

    mrb_define_const(mrb, rubyvm_task_class, "QUEUED", mrb_fixnum_value(queue_task_queued));
    mrb_define_const(mrb, rubyvm_task_class, "RUNNING", mrb_fixnum_value(queue_task_running));
    mrb_define_const(mrb, rubyvm_task_class, "FINISHED", mrb_fixnum_value(queue_task_finished));
}
