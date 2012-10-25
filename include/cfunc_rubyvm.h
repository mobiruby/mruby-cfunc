//
//  CFunc::RubyVM class
// 
//  See Copyright Notice in cfunc.h
//


#ifndef cfunc_rubyvm_h
#define cfunc_rubyvm_h

#include "cfunc.h"
#include "vector.h"
#include <pthread.h>

struct cfunc_rubyvm_data {
    mrb_state *state;
    const char* mrb_data;

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    vector_p queue;
};

void init_cfunc_rubyvm(mrb_state *mrb, struct RClass* module);

#endif
