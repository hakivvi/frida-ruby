#pragma once

#include "c_frida.h"

extern VALUE _gvl_bridge_thread;

typedef struct {
    GClosure	gclosure;
    bool		is_lambda;
    uint		signal_id;
    int         arity;
} RBClosure;

typedef struct {
    GClosure *closure;
    guint n_param_values;
    GValue *param_values;
} gclosure_callback;

typedef struct {
    GTask   *task;
    FridaRBAuthenticationService *self;
} gtask_callback;

typedef enum {
    GCLOSURE,
    GTASK
} callback_type;

typedef struct {
    callback_type type;
    union {
        gclosure_callback GC;
        gtask_callback GT;
    };
} gvl_bridge_data;

#define RET_IF_MAIN_THREAD_EXITED if (main_thread_exited) return (NULL);

void	gvl_bridge(void);
void	gvl_bridge_forward_GC(GClosure *closure, GValue *_, guint n_param_values, GValue *param_values, gpointer __, gpointer ___);
void	gvl_bridge_forward_GT(GTask *task, FridaRBAuthenticationService *self);
