#include "gvl_bridge.h"

static int	_gvl_bridge_pipe[2] = {-1, -1};
static int	__gvl_bridge_pipe[2] = {-1, -1};
static bool main_thread_exited = false;

static void gvl_bridge_signal_exit(VALUE _)
{
    main_thread_exited = true;
    close(_gvl_bridge_pipe[0]);
    close(_gvl_bridge_pipe[1]);
    close(__gvl_bridge_pipe[0]);
    close(__gvl_bridge_pipe[1]);
}

void	gvl_bridge_forward_GC(GClosure *closure, GValue *_, guint n_param_values, GValue *param_values, gpointer __, gpointer ___)
{
    void *dummy;
    gvl_bridge_data *data = malloc(sizeof(gvl_bridge_data));

    data->type = GCLOSURE;
    data->GC.closure = closure;
    data->GC.n_param_values = n_param_values;
    data->GC.param_values = param_values;
    write(_gvl_bridge_pipe[1], &data, sizeof(void *));
    // hold until the callback is done, also prevents GC-ing param_values
    read(__gvl_bridge_pipe[0], &dummy, sizeof(void *));
    if (data->GC.param_values != param_values)
        free(data->GC.param_values);
    free(data);
}

static VALUE GC_rbcall(gclosure_callback *callback)
{
    RBClosure *rc = TO_RBCLOSURE(callback->closure);
    VALUE *args_array;

    int arity = rc->is_lambda ? MIN(callback->n_param_values, rc->arity) : (callback->n_param_values - 1);
    if (arity == -1) // splat operator
        arity = callback->n_param_values;
    if (callback->n_param_values == arity)
        args_array = rbGObjectSignalClosure_marshal_params(callback->param_values, arity);
    else
        args_array = rbGObjectSignalClosure_marshal_params(callback->param_values + 1, arity); // +1 skip sender instance.
    callback->param_values = (void *)args_array;
    rb_funcallv((VALUE)callback->closure->data, rb_intern("call"), arity, args_array);
    return (Qnil);
}

static void gvl_bridge_call_GC(gclosure_callback *data)
{
    int state;

    rb_protect((VALUE (*)(VALUE))GC_rbcall, (VALUE)data, &state);
    if (state)
        rb_set_errinfo(Qnil);
}

void	gvl_bridge_forward_GT(GTask *task, FridaRBAuthenticationService *self)
{
    gvl_bridge_data *data = malloc(sizeof(gvl_bridge_data));

    data->type = GTASK;
    data->GT.task = task;
    data->GT.self = self;
    write(_gvl_bridge_pipe[1], &data, sizeof(void *));
}

static VALUE GT_rbcall(gtask_callback *data)
{
    gchar *token;

    token = g_task_get_task_data(data->task);
    return (rb_funcall((VALUE)data->self->callback, rb_intern("call"), 1, rb_str_new_cstr(token)));
}

static void gvl_bridge_call_GT(gtask_callback *data)
{
    int state;
    gchar *msg = NULL;
    VALUE result;

    result = rb_protect((VALUE (*)(VALUE))GT_rbcall, (VALUE)data, &state);
    if (state) {
        result = rb_funcall(rb_gv_get("$!"), rb_intern("message"), 0);
        if (result != Qnil)
            msg = g_strdup(StringValueCStr(result));
        else
            msg = g_strdup("Internal error");
        rb_set_errinfo(Qnil);
        g_task_return_new_error(data->task, FRIDA_ERROR, FRIDA_ERROR_INVALID_ARGUMENT, "%s", msg);
        g_free(msg);
    } else {
        rbGObject_unmarshal_string(result, &msg);
        g_task_return_pointer(data->task, msg, g_free);
    }
    g_object_unref(data->task);
}

static void gvl_bridge_main()
{
    gvl_bridge_data	*data;

    g_assert(sizeof(void *) <= sizeof(VALUE));
    while (1) {
        read(_gvl_bridge_pipe[0], &data, sizeof(void *));
        if (!data || main_thread_exited)
            return;
        if (data->type == GCLOSURE) {
            rb_thread_call_with_gvl((void_fp)gvl_bridge_call_GC, &data->GC);
            write(__gvl_bridge_pipe[1], &data, sizeof(void *));
        } else if (data->type == GTASK) {
            rb_thread_call_with_gvl((void_fp)gvl_bridge_call_GT, &data->GT);
            free(data);
        }
    }
}

void gvl_bridge(void)
{
    if (pipe(_gvl_bridge_pipe) < 0) {
        raise_rerror("pipe() error @ gvl_bridge", NULL);
        exit(EXIT_FAILURE);
    }
    if (pipe(__gvl_bridge_pipe) < 0) {
        raise_rerror("pipe() error @ gvl_bridge", NULL);
        exit(EXIT_FAILURE);
    }
    rb_set_end_proc(gvl_bridge_signal_exit, Qnil);
    rb_thread_call_without_gvl((void_fp)gvl_bridge_main, NULL, RUBY_UBF_IO, NULL);
}
