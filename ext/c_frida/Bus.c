#include "Bus.h"

VALUE	Bus_from_FridaBus(FridaBus *fridabus)
{
    VALUE	self;

    self = rb_class_new_instance(0, NULL, cBus);
    GET_GOBJECT_DATA();
    d->handle = fridabus;
    d->destroy = g_object_unref;
    return (self);
}

GVL_FREE_PROXY_FUNC(attach_sync, void *handle)
{
    GError *gerr = NULL;

    frida_bus_attach_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #attach() -> nil
*/
static VALUE Bus_attach(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, attach_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(post, bus_post_proxy_args *args)
{
    GError *gerr = NULL;

    frida_bus_post(args->handle, args->message, args->data);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #post(message, data:) -> nil
*/
static VALUE Bus_post(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE message, kws, data;
    GBytes *gdata = NULL;

    rb_scan_args(argc, argv, "1:", &message, &kws);
    if (!RB_TYPE_P(message, T_STRING)) {
        raise_argerror("message should be a string.");
        return (Qnil);
    }
    if (!NIL_P(kws)) {
        data = rb_hash_aref(kws, ID2SYM(rb_intern("data")));
        if (!NIL_P(data)) {
            if (!RB_TYPE_P(data, T_STRING)) {
                raise_argerror("scope must be a string.");
                return (Qnil);
            }
            gdata = g_bytes_new(RSTRING_PTR(data), RSTRING_LEN(data));
        }
    }
    bus_post_proxy_args args = {
        .handle = d->handle,
        .message = StringValueCStr(message),
        .data = gdata
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, post, &args);
    g_bytes_unref(gdata);
    return (Qnil);

gerror:
    g_bytes_unref(gdata);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

void	define_Bus()
{
    cBus = rb_define_class_under(mCFrida, "Bus", cGObject);
    rb_define_method(cBus, "attach", Bus_attach, 0);
    rb_define_method(cBus, "post", Bus_post, -1);
}
