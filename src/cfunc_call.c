//
//  CFunc::call method
// 
//  See Copyright Notice in cfunc.h
//

#include "cfunc_call.h"
#include "cfunc_pointer.h"
#include "cfunc_type.h"
#include "cfunc_utils.h"

#include "mruby/string.h"
#include "mruby/class.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdbool.h>
#include "ffi.h"

#ifdef _WIN32
#include <windows.h>
static void*
get_proc_address(const char* funcname)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    PBYTE pImage = (PBYTE) hInst;
    PIMAGE_DOS_HEADER pDOS = (PIMAGE_DOS_HEADER) hInst;
    PIMAGE_NT_HEADERS pPE;
    PIMAGE_IMPORT_DESCRIPTOR pImpDesc;

    if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
        return NULL;
    pPE = (PIMAGE_NT_HEADERS)(pImage + pDOS->e_lfanew);
    if (pPE->Signature != IMAGE_NT_SIGNATURE)
        return NULL;
    pImpDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pImage
        + pPE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; pImpDesc->FirstThunk; ++pImpDesc) {
        HINSTANCE hLib = LoadLibrary((const char*)(pImage + pImpDesc->Name));
        if (hLib) {
            void* p = GetProcAddress(hLib, "strcpy");
            if (p) return p;
        }
    }
    return NULL;
}
#endif

static mrb_value
cfunc_call(mrb_state *mrb, mrb_value self)
{
    int margc;
    mrb_value mresult_type, mname, *margs;
    void **values = NULL;
    ffi_type **args = NULL;

    mrb_get_args(mrb, "oo*", &mresult_type, &mname, &margs, &margc);

    void *fp = NULL;
    if(mrb_string_p(mname) || mrb_symbol_p(mname)) {
#ifndef _WIN32
        void *dlh = dlopen(NULL, RTLD_LAZY);
        fp = dlsym(dlh, mrb_string_value_ptr(mrb, mname));
        dlclose(dlh);
#else
        fp = get_proc_address(mrb_string_value_ptr(mrb, mname));
#endif

        if(fp == NULL) {
            mrb_raisef(mrb, E_NAME_ERROR, "can't find C function %s", mrb_string_value_ptr(mrb, mname));
            goto cfunc_call_exit;
        }
    }
    else {
        fp = cfunc_pointer_ptr(mname);
        if(fp == NULL) {
            mrb_raisef(mrb, E_NAME_ERROR, "can't call NULL pointer");
            goto cfunc_call_exit;
        }
    }

    args = malloc(sizeof(ffi_type*) * margc);
    values = malloc(sizeof(void*) * margc);
    mrb_sym sym_to_ffi_value = mrb_intern(mrb, "to_ffi_value");

    mrb_value nil_ary[1];
    nil_ary[0] = mrb_nil_value();
    int i;
    for(i = 0; i < margc; ++i) {
        if(mrb_respond_to(mrb, margs[i], sym_to_ffi_value)) {
            args[i] = mrb_value_to_mrb_ffi_type(mrb, margs[i])->ffi_type_value;
            values[i] = cfunc_pointer_ptr(mrb_funcall_argv(mrb, margs[i], sym_to_ffi_value, 1, nil_ary));
        }
        else {
            cfunc_mrb_raise_without_jump(mrb, E_TYPE_ERROR, "ignore argument type %s", mrb_obj_classname(mrb, margs[i]));
            goto cfunc_call_exit;
        }
    }
    
    ffi_type *result_type = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(mresult_type))->ffi_type_value;
    if (result_type == NULL) {
        cfunc_mrb_raise_without_jump(mrb, E_ARGUMENT_ERROR, "ignore return type %s", mrb_class_name(mrb, mrb_class_ptr(mresult_type)));
        goto cfunc_call_exit;
    }
    
    mrb_value mresult = mrb_nil_value();
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, margc, result_type, args) == FFI_OK) {
        void *result;
        if(result_type->size > sizeof(long)) {
            result = malloc(result_type->size);
        }
        else if(result_type->size) {
            result = malloc(sizeof(long));
        }
        else {
            result = NULL;
        }
        ffi_call(&cif, fp, result, values);
        
        if(result) {
            mrb_value result_ptr = cfunc_pointer_new_with_pointer(mrb, result, true);
            mresult = mrb_funcall(mrb, mresult_type, "refer", 1, result_ptr);
        }
    }
    else {
        mrb_raisef(mrb, E_NAME_ERROR, "Can't find C function %s", mname);
        goto cfunc_call_exit;
    }

cfunc_call_exit:
    free(values);
    free(args);
    return mresult;
}


static mrb_value
cfunc_libcall(mrb_state *mrb, mrb_value self)
{
    int margc;
    mrb_value mresult_type, mlib, mname, *margs;
    void **values = NULL;
    ffi_type **args = NULL;

    mrb_get_args(mrb, "oSo*", &mresult_type, &mlib, &mname, &margs, &margc);

    void *fp = NULL;
    if((mrb_string_p(mname) || mrb_symbol_p(mname))) {
        void *dlh = dlopen(mrb_string_value_ptr(mrb, mlib), RTLD_LAZY);
        fp = dlsym(dlh, mrb_string_value_ptr(mrb, mname));
        dlclose(dlh);

        if(fp == NULL) {
            mrb_raisef(mrb, E_NAME_ERROR, "can't find C function %s", mrb_string_value_ptr(mrb, mname));
            goto cfunc_call_exit;
        }
    }
    else {
        fp = cfunc_pointer_ptr(mname);
        if(fp == NULL) {
            mrb_raisef(mrb, E_NAME_ERROR, "can't call NULL pointer");
            goto cfunc_call_exit;
        }
    }

    args = malloc(sizeof(ffi_type*) * margc);
    values = malloc(sizeof(void*) * margc);
    mrb_sym sym_to_ffi_value = mrb_intern(mrb, "to_ffi_value");

    mrb_value nil_ary[1];
    nil_ary[0] = mrb_nil_value();
    int i;
    for(i = 0; i < margc; ++i) {
        if(mrb_respond_to(mrb, margs[i], sym_to_ffi_value)) {
            args[i] = mrb_value_to_mrb_ffi_type(mrb, margs[i])->ffi_type_value;
            values[i] = cfunc_pointer_ptr(mrb_funcall_argv(mrb, margs[i], sym_to_ffi_value, 1, nil_ary));
        }
        else {
            cfunc_mrb_raise_without_jump(mrb, E_TYPE_ERROR, "ignore argument type %s", mrb_obj_classname(mrb, margs[i]));
            goto cfunc_call_exit;
        }
    }

    ffi_type *result_type = rclass_to_mrb_ffi_type(mrb, mrb_class_ptr(mresult_type))->ffi_type_value;
    if (result_type == NULL) {
        cfunc_mrb_raise_without_jump(mrb, E_ARGUMENT_ERROR, "ignore return type %s", mrb_class_name(mrb, mrb_class_ptr(mresult_type)));
        goto cfunc_call_exit;
    }

    mrb_value mresult = mrb_nil_value();
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, margc, result_type, args) == FFI_OK) {
        void *result;
        if(result_type->size > sizeof(long)) {
            result = malloc(result_type->size);
        }
        else if(result_type->size) {
            result = malloc(sizeof(long));
        }
        else {
            result = NULL;
        }
        ffi_call(&cif, fp, result, values);

        if(result) {
            mrb_value result_ptr = cfunc_pointer_new_with_pointer(mrb, result, true);
            mresult = mrb_funcall(mrb, mresult_type, "refer", 1, result_ptr);
        }
    }
    else {
        mrb_raisef(mrb, E_NAME_ERROR, "Can't find C function %s", mname);
        goto cfunc_call_exit;
    }

cfunc_call_exit:
    free(values);
    free(args);
    return mresult;
}


void init_cfunc_call(mrb_state *mrb, struct RClass* module)
{
    mrb_define_class_method(mrb, module, "call", cfunc_call, ARGS_ANY());
    mrb_define_module_function(mrb, module, "libcall", cfunc_libcall, ARGS_ANY());
}
