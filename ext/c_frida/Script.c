#include "Script.h"

VALUE	Script_from_FridaScript(FridaScript *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cScript);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = frida_unref;
    return (self);
}

/*
    call-seq:
        #is_destroyed() -> [TrueClass, FalseClass]
*/
static VALUE Script_is_destroyed(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    void	*is_destroyed;

    is_destroyed = rb_thread_call_without_gvl((void_fp)frida_script_is_destroyed, d->handle, NULL, NULL);
    return (is_destroyed ? Qtrue : Qfalse);
}

GVL_FREE_PROXY_FUNC(load_sync, void *handle)
{
    GError *gerr = NULL;

    frida_script_load_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #load() -> nil
*/
static VALUE Script_load(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, load_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(eternalize_sync, void *handle)
{
    GError *gerr = NULL;

    frida_script_eternalize_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #eternalize() -> nil
*/
static VALUE Script_eternalize(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, eternalize_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(unload_sync, void *handle)
{
    GError *gerr = NULL;

    frida_script_unload_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #unload() -> nil
*/
static VALUE Script_unload(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, unload_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(disable_debugger_sync, void *handle)
{
    GError *gerr = NULL;

    frida_script_disable_debugger_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #disable_debugger() -> nil
*/
static VALUE Script_disable_debugger(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, disable_debugger_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(enable_debugger_sync, enable_debugger_proxy_args *args)
{
    GError *gerr = NULL;

    frida_script_enable_debugger_sync(args->handle, args->port, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #enable_debugger(port:) -> nil
*/
static VALUE Script_enable_debugger(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE kws, rport;
    uint port = 0;

    rb_scan_args(argc, argv, ":", &kws);
    if (!NIL_P(kws)) {
        rport = rb_hash_aref(kws, ID2SYM(rb_intern("port")));
        if (!NIL_P(rport)) {
            if (!RB_TYPE_P(rport, T_FIXNUM)) {
                raise_argerror("port must be a number.");
                return (Qnil);
            }
            port = NUM2UINT(rport);
        }
    }
    enable_debugger_proxy_args args = {
        .handle = d->handle,
        .port = port
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, enable_debugger_sync, &args);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(post, post_proxy_args *args)
{
    GError *gerr = NULL;

    frida_script_post(args->handle, args->message, args->data);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #post(message, data:) -> nil
*/
static VALUE Script_post(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE kws, msg, data;
    gpointer cdata = NULL;

    rb_scan_args(argc, argv, "1:", &msg, &kws);
    if (!NIL_P(kws)) {
        data = rb_hash_aref(kws, ID2SYM(rb_intern("data")));
        if (!NIL_P(data)) {
            if (!RB_TYPE_P(data, T_STRING)) {
                raise_argerror("data must be a number.");
                return (Qnil);
            }
            cdata = g_bytes_new(RSTRING_PTR(data), RSTRING_LEN(data));
        }
    }
    post_proxy_args args = {
        .handle = d->handle,
        .message = StringValueCStr(msg),
        .data = cdata
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, post, &args);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    g_bytes_unref(args.data);
    return (Qnil);
}

void	define_Script()
{
    cScript = rb_define_class_under(mCFrida, "Script", cGObject);

    rb_define_method(cScript, "is_destroyed", Script_is_destroyed, 0);
    rb_define_method(cScript, "load", Script_load, 0);
    rb_define_method(cScript, "unload", Script_unload, 0);
    rb_define_method(cScript, "eternalize", Script_eternalize, 0);
    rb_define_method(cScript, "enable_debugger", Script_enable_debugger, -1);
    rb_define_method(cScript, "disable_debugger", Script_disable_debugger, 0);
    rb_define_method(cScript, "post", Script_post, -1);
}
