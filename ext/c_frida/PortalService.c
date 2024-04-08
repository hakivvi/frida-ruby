#include "PortalService.h"

GVL_FREE_PROXY_FUNC(stop_sync, FridaPortalService *handle)
{
    GError *gerr = NULL;

    frida_portal_service_stop_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

static void PortalService_destroy(void *handle)
{
    CALL_GVL_FREE_WITH_RET(void *dummy, stop_sync, handle);
    frida_unref(handle);
    return ;

gerror:
    g_error_free(_gerr);
    frida_unref(handle);
}

/*
    call-seq:
        #device() -> Device
*/
static VALUE PortalService_device(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("device")));
}

/*
    call-seq:
        #stop() -> nil
*/
static VALUE PortalService_stop(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, stop_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}


GVL_FREE_PROXY_FUNC(start_sync, FridaPortalService *handle)
{
    GError *gerr = NULL;

    frida_portal_service_start_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #start() -> nil
*/
static VALUE PortalService_start(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, start_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(kick, kick_proxy_args *args)
{
    GError *gerr = NULL;

    frida_portal_service_kick(args->handle, args->connection_id);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #kick(connection_id) -> nil
*/
static VALUE PortalService_kick(VALUE self, VALUE connection_id)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(connection_id, T_FIXNUM)) {
        raise_argerror("connection_id must be an integer.");
        return (Qnil);
    }
    kick_proxy_args args = {
        .handle = d->handle,
        .connection_id = NUM2UINT(connection_id)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, kick, &args);
    return (Qnil);

    UNREACHABLE_GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(post, _post_proxy_args *args)
{
    GError *gerr = NULL;

    frida_portal_service_post(args->handle, args->connection_id, args->message, args->data);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #post(connection_id, message, data:) -> nil
*/
static VALUE PortalService_post(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE connection_id, message, kws;
    GBytes *gdata = NULL;

    rb_scan_args(argc, argv, "2:", &connection_id, &message, &kws);
    if (!RB_TYPE_P(connection_id, T_FIXNUM)) {
        raise_argerror("tag must be a number.");
        return (Qnil);
    }
    if (!RB_TYPE_P(message, T_STRING)) {
        raise_argerror("message must be a String.");
        return (Qnil);
    }
    if (!NIL_P(kws)) {
        VALUE data = rb_hash_aref(kws, ID2SYM(rb_intern("data")));
        if (!NIL_P(data)) {
            if (!RB_TYPE_P(data, T_STRING)) {
                raise_argerror("data must be a String.");
                return (Qnil);
            }
        }
        gdata = g_bytes_new(RSTRING_PTR(data), RSTRING_LEN(data));
    }
    _post_proxy_args args = {
        .handle = d->handle,
        .connection_id = NUM2UINT(connection_id),
        .message = StringValueCStr(message),
        .data = gdata
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, post, &args);
    g_bytes_unref(gdata);
    return (Qnil);

    UNREACHABLE_GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(narrowcast, narrowcast_proxy_args *args)
{
    GError *gerr = NULL;

    frida_portal_service_narrowcast(args->handle, args->tag, args->message, args->data);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #narrowcast(tag, message, data:) -> nil
*/
static VALUE PortalService_narrowcast(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE tag, message, kws;
    GBytes *gdata = NULL;

    rb_scan_args(argc, argv, "2:", &tag, &message, &kws);
    if (!RB_TYPE_P(tag, T_STRING)) {
        raise_argerror("tag must be a String.");
        return (Qnil);
    }
    if (!RB_TYPE_P(message, T_STRING)) {
        raise_argerror("message must be a String.");
        return (Qnil);
    }
    if (!NIL_P(kws)) {
        VALUE data = rb_hash_aref(kws, ID2SYM(rb_intern("data")));
        if (!NIL_P(data)) {
            if (!RB_TYPE_P(data, T_STRING)) {
                raise_argerror("data must be a String.");
                return (Qnil);
            }
        }
        gdata = g_bytes_new(RSTRING_PTR(data), RSTRING_LEN(data));
    }
    narrowcast_proxy_args args = {
        .handle = d->handle,
        .tag = StringValueCStr(tag),
        .message = StringValueCStr(message),
        .data = gdata
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, narrowcast, &args);
    g_bytes_unref(gdata);
    return (Qnil);

    UNREACHABLE_GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(broadcast, broadcast_proxy_args *args)
{
    GError *gerr = NULL;

    frida_portal_service_broadcast(args->handle, args->message, args->data);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #broadcast(message, data:) -> nil
*/
static VALUE PortalService_broadcast(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE message, kws;
    GBytes *gdata = NULL;

    rb_scan_args(argc, argv, "1:", &message, &kws);
    if (!RB_TYPE_P(message, T_STRING)) {
        raise_argerror("message must be a String.");
        return (Qnil);
    }
    if (!NIL_P(kws)) {
        VALUE data = rb_hash_aref(kws, ID2SYM(rb_intern("data")));
        if (!NIL_P(data)) {
            if (!RB_TYPE_P(data, T_STRING)) {
                raise_argerror("data must be a String.");
                return (Qnil);
            }
        }
        gdata = g_bytes_new(RSTRING_PTR(data), RSTRING_LEN(data));
    }
    broadcast_proxy_args args = {
        .handle = d->handle,
        .message = StringValueCStr(message),
        .data = gdata
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, broadcast, &args);
    g_bytes_unref(gdata);
    return (Qnil);

    UNREACHABLE_GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(enumerate_tags, enumerate_tags_proxy_args *args)
{
    GError *gerr = NULL;
    gchar **tags;

    tags = frida_portal_service_enumerate_tags(args->handle, args->connection_id, args->tags_count);
    RETURN_GVL_FREE_RESULT(tags);
}

/*
    call-seq:
        #enumerate_tags(connection_id) -> Array
*/
static VALUE PortalService_enumerate_tags(VALUE self, VALUE connection_id)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    gint tags_count;

    if (!RB_TYPE_P(connection_id, T_FIXNUM)) {
        raise_argerror("connection_id must be an integer.");
        return (Qnil);
    }
    enumerate_tags_proxy_args args = {
        .handle = d->handle,
        .connection_id = NUM2UINT(connection_id),
        .tags_count = &tags_count
    };
    CALL_GVL_FREE_WITH_RET(gchar **tags, enumerate_tags, &args);
    VALUE rtags = rbGObject_marshal_strv(tags, tags_count);
    g_strfreev(tags);
    return (rtags);

    UNREACHABLE_GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(tag, tag_proxy_args *args)
{
    GError *gerr = NULL;

    frida_portal_service_tag(args->handle, args->connection_id, args->tag);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #tag(connection_id, tag) -> nil
*/
static VALUE PortalService_tag(VALUE self, VALUE connection_id, VALUE tag)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(connection_id, T_FIXNUM)) {
        raise_argerror("connection_id must be an integer.");
        return (Qnil);
    }
    if (!RB_TYPE_P(tag, T_STRING)) {
        raise_argerror("tag must be a String.");
        return (Qnil);
    }
    tag_proxy_args args = {
        .handle = d->handle,
        .connection_id = NUM2UINT(connection_id),
        .tag = StringValueCStr(tag)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, tag, &args);
    return (Qnil);

    UNREACHABLE_GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(untag, untag_proxy_args *args)
{
    GError *gerr = NULL;

    frida_portal_service_untag(args->handle, args->connection_id, args->tag);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #untag(connection_id, tag) -> nil
*/
static VALUE PortalService_untag(VALUE self, VALUE connection_id, VALUE tag)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(connection_id, T_FIXNUM)) {
        raise_argerror("connection_id must be an integer.");
        return (Qnil);
    }
    if (!RB_TYPE_P(tag, T_STRING)) {
        raise_argerror("tag must be a String.");
        return (Qnil);
    }
    untag_proxy_args args = {
        .handle = d->handle,
        .connection_id = NUM2UINT(connection_id),
        .tag = StringValueCStr(tag)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, untag, &args);
    return (Qnil);

    UNREACHABLE_GERROR_BLOCK
}

/*
    call-seq:
        #new(cluster_params, control_params:) -> PortalService
*/
static VALUE PortalService_initialize(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    VALUE cluster_params, kws;
    FridaEndpointParameters *c, *cc = NULL;

    rb_scan_args(argc, argv, "1:", &cluster_params, &kws);
    if (!rb_obj_is_instance_of(cluster_params, cEndpointParameters)) {
        raise_argerror("cluster_params must be an instance of EndpointParameters.");
        return (Qnil);
    }
    GET_DATA_EX(GObject, cluster_params, c_d);
    c = c_d->handle;
    // we support creating classes without a handle for marshaling purposes, but those instances should not work here.
    if (!c) {
        raise_argerror("cluster_params must be an instance of a valid EndpointParameters.");
        return (Qnil);
    }
    if (!NIL_P(kws)) {
        VALUE control_params = rb_hash_aref(kws, ID2SYM(rb_intern("control_params")));
        if (!NIL_P(control_params)) {
            if (!rb_obj_is_instance_of(control_params, cEndpointParameters)) {
                raise_argerror("control_params must be an instance of EndpointParameters.");
                return (Qnil);
            }
            GET_DATA_EX(GObject, control_params, cc_d);
            cc = cc_d->handle;
        }
    }
    FridaPortalService *handle = frida_portal_service_new(c, cc);
    d->handle = handle;
    d->destroy = PortalService_destroy;
    rb_ivar_set(self, rb_intern("device"), Device_from_FridaDevice(g_object_ref(frida_portal_service_get_device(d->handle))));
    return (self);
}

void	define_PortalService()
{
    cPortalService = rb_define_class_under(mCFrida, "PortalService", cGObject);

    rb_define_method(cPortalService, "initialize", PortalService_initialize, -1);
    rb_define_method(cPortalService, "device", PortalService_device, 0);

    rb_define_method(cPortalService, "start", PortalService_start, 0);
    rb_define_method(cPortalService, "stop", PortalService_stop, 0);
    rb_define_method(cPortalService, "kick", PortalService_kick, 1);
    rb_define_method(cPortalService, "post", PortalService_post, -1);
    rb_define_method(cPortalService, "narrowcast", PortalService_narrowcast, -1);
    rb_define_method(cPortalService, "broadcast", PortalService_broadcast, -1);
    rb_define_method(cPortalService, "enumerate_tags", PortalService_enumerate_tags, 1);
    rb_define_method(cPortalService, "tag", PortalService_tag, 2);
    rb_define_method(cPortalService, "untag", PortalService_untag, 2);
}
