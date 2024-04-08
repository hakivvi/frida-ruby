#include "Session.h"

VALUE	Session_from_FridaSession(FridaSession *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cSession);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = frida_unref;
    rb_ivar_set(self, rb_intern("pid"), UINT2NUM(frida_session_get_pid(d->handle)));
    return (self);
}

static VALUE Session_inspect(VALUE self)
{
    VALUE s;

    s = rb_sprintf("#<Session: pid=%+"PRIsVALUE">", rb_funcall(self, rb_intern("pid"), 0, NULL));
    return (s);
}

/*
    call-seq:
        #pid() -> Fixnum
*/
static VALUE Session_pid(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("pid")));
}

GVL_FREE_PROXY_FUNC(detach_sync, void *handle)
{
    GError *gerr = NULL;

    frida_session_detach_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #detach() -> nil
*/
static VALUE Session_detach(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, detach_sync, d->handle);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(resume_sync, void *handle)
{
    GError *gerr = NULL;

    frida_session_resume_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #resume() -> nil
*/
static VALUE Session_resume(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, resume_sync, d->handle);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    return (Qnil);
}

/*
    call-seq:
        #is_detached() -> [TrueClass, FalseClass]
*/
static VALUE Session_is_detached(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    void	*is_detached;

    is_detached = rb_thread_call_without_gvl((void_fp)frida_session_is_detached, d->handle, NULL, NULL);
    return (is_detached ? Qtrue : Qfalse);
}

GVL_FREE_PROXY_FUNC(enable_child_gating_sync, void *handle)
{
    GError *gerr = NULL;

    frida_session_enable_child_gating_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #enable_child_gating() -> nil
*/
static VALUE Session_enable_child_gating(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, enable_child_gating_sync, d->handle);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(disable_child_gating_sync, void *handle)
{
    GError *gerr = NULL;

    frida_session_disable_child_gating_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #disable_child_gating() -> nil
*/
static VALUE Session_disable_child_gating(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, disable_child_gating_sync, d->handle);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(create_script_sync, create_script_sync_proxy_args *args)
{
    GError *gerr = NULL;
    FridaScript *handle;

    handle = frida_session_create_script_sync(args->handle, args->source, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(handle);
}

/*
    call-seq:
        #create_script(source, name:, snapshot:, runtime:) -> Script
*/
static VALUE Session_create_script(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE source, kws;

    rb_scan_args(argc, argv, "1:", &source, &kws);
    if (!RB_TYPE_P(source, T_STRING)) {
        raise_argerror("source must be a string.");
        return (Qnil);
    }
    FridaScriptOptions *options = frida_script_options_new();
    if (!NIL_P(kws)) {
        VALUE name = rb_hash_aref(kws, ID2SYM(rb_intern("name")));
        if (!NIL_P(name)) {
            if (!RB_TYPE_P(name, T_STRING)) {
                raise_argerror("name must be a string.");
                return (Qnil);
            }
            frida_script_options_set_name(options, StringValueCStr(name));
        }
        VALUE snapshot = rb_hash_aref(kws, ID2SYM(rb_intern("snapshot")));
        if (!NIL_P(snapshot)) {
            if (!RB_TYPE_P(snapshot, T_STRING)) {
                raise_argerror("snapshot must be a string.");
                return (g_object_unref(options), Qnil);
            }
            GBytes *gsnapshot = g_bytes_new(RSTRING_PTR(snapshot), RSTRING_LEN(snapshot));
            frida_script_options_set_snapshot(options, gsnapshot);
        }
        VALUE runtime = rb_hash_aref(kws, ID2SYM(rb_intern("runtime")));
        if (!NIL_P(runtime)) {
            if (!RB_TYPE_P(runtime, T_STRING)) {
                raise_argerror("runtime must be a string.");
                return (g_object_unref(options), Qnil);
            }
            FridaScriptRuntime gruntime;
            if (!rbGObject_unmarshal_enum(StringValueCStr(runtime), FRIDA_TYPE_SCRIPT_RUNTIME, &gruntime)) {
                raise_argerror("invalid runtime.");
                return (g_object_unref(options), Qnil);
            }
            frida_script_options_set_runtime(options, gruntime);
        }
    }
    create_script_sync_proxy_args args = {
        .handle = d->handle,
        .source = StringValueCStr(source),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(FridaScript *script, create_script_sync, &args);
    g_clear_object(&options);
    return (Script_from_FridaScript(script));

gerror:
    g_clear_object(&options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(create_script_from_bytes_sync, create_script_from_bytes_sync_proxy_args *args)
{
    GError *gerr = NULL;
    FridaScript *handle;

    handle = frida_session_create_script_from_bytes_sync(args->handle, args->bytes, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(handle);
}

/*
    call-seq:
        #create_script_from_bytes(data, name:, snapshot:, runtime:) -> Script
*/
static VALUE Session_create_script_from_bytes(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaScriptOptions *options;
    GBytes *cdata;
    VALUE data, kws;

    rb_scan_args(argc, argv, "1:", &data, &kws);
    if (!RB_TYPE_P(data, T_STRING)) {
        raise_argerror("data must be a string.");
        return (Qnil);
    }
    cdata = g_bytes_new(RSTRING_PTR(data), RSTRING_LEN(data));
    options = frida_script_options_new();
    if (!NIL_P(kws)) {
        VALUE name = rb_hash_aref(kws, ID2SYM(rb_intern("name")));
        if (!NIL_P(name)) {
            if (!RB_TYPE_P(name, T_STRING)) {
                raise_argerror("name must be a string.");
                goto invalid_arg;
            }
            frida_script_options_set_name(options, StringValueCStr(name));
        }
        VALUE snapshot = rb_hash_aref(kws, ID2SYM(rb_intern("snapshot")));
        if (!NIL_P(snapshot)) {
            if (!RB_TYPE_P(snapshot, T_STRING)) {
                raise_argerror("snapshot must be a string.");
                goto invalid_arg;
            }
            GBytes *csnapshot = g_bytes_new(RSTRING_PTR(snapshot), RSTRING_LEN(snapshot));
            frida_script_options_set_snapshot(options, csnapshot);
            g_bytes_unref(csnapshot);
        }
        VALUE rruntime = rb_hash_aref(kws, ID2SYM(rb_intern("runtime")));
        if (!NIL_P(rruntime)) {
            if (!RB_TYPE_P(rruntime, T_STRING)) {
                raise_argerror("runtime must be a string.");
                goto invalid_arg;
            }
            FridaScriptRuntime runtime;
            if (!rbGObject_unmarshal_enum(StringValueCStr(rruntime), FRIDA_TYPE_SCRIPT_RUNTIME, &runtime)) {
                raise_argerror("runtime is invalid.");
                goto invalid_arg;
            }
            frida_script_options_set_runtime(options, runtime);
        }
    }
    create_script_from_bytes_sync_proxy_args args = {
        .handle = d->handle,
        .bytes = cdata,
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(FridaScript *script, create_script_from_bytes_sync, &args);
    g_bytes_unref(cdata);
    g_clear_object(&options);
    return (Script_from_FridaScript(script));

gerror:
    g_clear_object(&options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
invalid_arg:
    g_object_unref(options);
    g_bytes_unref(cdata);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(compile_script_sync, compile_script_sync_proxy_args *args)
{
    GError *gerr = NULL;
    GBytes *result_bytes;

    result_bytes = frida_session_compile_script_sync(args->handle, args->source, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(result_bytes);
}

/*
    call-seq:
        #compile_script(source, name:, runtime:) -> String
*/
static VALUE Session_compile_script(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaScriptOptions *options;
    VALUE source, kws;

    rb_scan_args(argc, argv, "1:", &source, &kws);
    if (!RB_TYPE_P(source, T_STRING)) {
        raise_argerror("source must be a string.");
        return (Qnil);
    }
    options = frida_script_options_new();
    if (!NIL_P(kws)) {
        VALUE name = rb_hash_aref(kws, ID2SYM(rb_intern("name")));
        if (!NIL_P(name)) {
            if (!RB_TYPE_P(name, T_STRING)) {
                raise_argerror("name must be a string.");
                goto invalid_arg;
            }
            frida_script_options_set_name(options, StringValueCStr(name));
        }
        VALUE rruntime = rb_hash_aref(kws, ID2SYM(rb_intern("runtime")));
        if (!NIL_P(rruntime)) {
            if (!RB_TYPE_P(rruntime, T_STRING)) {
                raise_argerror("runtime must be a string.");
                goto invalid_arg;
            }
            FridaScriptRuntime runtime;
            if (!rbGObject_unmarshal_enum(StringValueCStr(rruntime), FRIDA_TYPE_SCRIPT_RUNTIME, &runtime)) {
                raise_argerror("runtime is invalid.");
                goto invalid_arg;
            }
            frida_script_options_set_runtime(options, runtime);
        }
    }
    compile_script_sync_proxy_args args = {
        .handle = d->handle,
        .source = StringValueCStr(source),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(GBytes *result_bytes, compile_script_sync, &args);
    g_clear_object(&options);
    VALUE rresult = rbGObject_marshal_bytes(result_bytes);
    g_bytes_unref(result_bytes);
    return (rresult);

gerror:
    g_clear_object(&options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
invalid_arg:
    g_object_unref(options);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(snapshot_script_sync, snapshot_script_sync_proxy_args *args)
{
    GError *gerr = NULL;
    GBytes *result_bytes;

    result_bytes = frida_session_snapshot_script_sync(args->handle, args->embed_script, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(result_bytes);
}

/*
    call-seq:
        #snapshot_script(embed_script, warmup_script:, runtime:) -> String
*/
static VALUE Session_snapshot_script(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaSnapshotOptions *options;
    VALUE embed_script, kws;

    rb_scan_args(argc, argv, "1:", &embed_script, &kws);
    if (!RB_TYPE_P(embed_script, T_STRING)) {
        raise_argerror("embed_script must be a string.");
        return (Qnil);
    }
    options = frida_snapshot_options_new();
    if (!NIL_P(kws)) {
        VALUE warmup_script = rb_hash_aref(kws, ID2SYM(rb_intern("warmup_script")));
        if (!NIL_P(warmup_script)) {
            if (!RB_TYPE_P(warmup_script, T_STRING)) {
                raise_argerror("warmup_script must be a string.");
                goto invalid_arg;
            }
            frida_snapshot_options_set_warmup_script(options, StringValueCStr(warmup_script));
        }
        VALUE rruntime = rb_hash_aref(kws, ID2SYM(rb_intern("runtime")));
        if (!NIL_P(rruntime)) {
            if (!RB_TYPE_P(rruntime, T_STRING)) {
                raise_argerror("runtime must be a string.");
                goto invalid_arg;
            }
            FridaScriptRuntime runtime;
            if (!rbGObject_unmarshal_enum(StringValueCStr(rruntime), FRIDA_TYPE_SCRIPT_RUNTIME, &runtime)) {
                raise_argerror("runtime is invalid.");
                goto invalid_arg;
            }
            frida_snapshot_options_set_runtime(options, runtime);
        }
    }
    snapshot_script_sync_proxy_args args = {
        .handle = d->handle,
        .embed_script = StringValueCStr(embed_script),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(GBytes *result_bytes, snapshot_script_sync, &args);
    g_clear_object(&options);
    VALUE rresult = rbGObject_marshal_bytes(result_bytes);
    g_bytes_unref(result_bytes);
    return (rresult);

gerror:
    g_clear_object(&options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
invalid_arg:
    g_object_unref(options);
    return (Qnil);
}

static int array_all_instance_of(VALUE arr, VALUE rtype)
{
    long len = RARRAY_LEN(arr);

    for (uint i = 0; i < len; i++) {
        if (!rb_obj_is_instance_of(RARRAY_AREF(arr, i), rtype))
            return (0);
    }
    return (1);
}

GVL_FREE_PROXY_FUNC(setup_peer_connection_sync, setup_peer_connection_sync_proxy_args *args)
{
    GError *gerr = NULL;

    frida_session_setup_peer_connection_sync(args->handle, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #setup_peer_connection(stun_server:, relays:) -> nil
*/
static VALUE Session_setup_peer_connection(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaPeerOptions *options;
    VALUE kws, stun_server, relays;

    options = frida_peer_options_new();
    rb_scan_args(argc, argv, ":", &kws);
    if (!NIL_P(kws)) {
        stun_server = rb_hash_aref(kws, ID2SYM(rb_intern("stun_server")));
        if (!NIL_P(stun_server)) {
            if (!RB_TYPE_P(stun_server, T_STRING)) {
                raise_argerror("stun_server must be a string.");
                goto invalid_arg;
            }
            frida_peer_options_set_stun_server(options, StringValueCStr(stun_server));
        } else {
            raise_argerror("expected stun_server.");
            goto invalid_arg;
        }
        relays = rb_hash_aref(kws, ID2SYM(rb_intern("relays")));
        if (!NIL_P(relays)) {
            if (!RB_TYPE_P(relays, T_ARRAY) || !array_all_instance_of(relays, cRelay)) {
                raise_argerror("relays must be an array of Relay.");
                goto invalid_arg;
            }
            GObject_d *relay_data;
            uint relays_count = RARRAY_LEN(relays);
            for (uint i = 0; i < relays_count; i++) {
                TypedData_Get_Struct(RARRAY_AREF(relays, i), GObject_d, &GObject_dt, relay_data);
                if (!relay_data->handle) {
                    raise_argerror("provided an invalid Relay object.");
                    goto invalid_arg;
                }
                frida_peer_options_add_relay(options, relay_data->handle);
            }
        } else {
            raise_argerror("expected relays array.");
            goto invalid_arg;
        }
    } else {
        raise_argerror("expected stun_server and relays.");
        goto invalid_arg;
    }
    setup_peer_connection_sync_proxy_args args = {
        .handle = d->handle,
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, setup_peer_connection_sync, &args);
    g_clear_object(&options);
    return (Qnil);

gerror:
    g_clear_object(&options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
invalid_arg:
    g_object_unref(options);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(join_portal_sync, join_portal_sync_proxy_args *args)
{
    GError *gerr = NULL;

    FridaPortalMembership *handle = frida_session_join_portal_sync(args->handle, args->address, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(handle);
}

/*
    call-seq:
        #join_portal(address, certificate:, token:, acl:) -> PortalMembership
*/
static VALUE Session_join_portal(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaPortalOptions *options;
    VALUE kws, address, certificate, token, acl;

    rb_scan_args(argc, argv, "1:", &address, &kws);
    if (!RB_TYPE_P(address, T_STRING)) {
        raise_argerror("address must be a string.");
        return (Qnil);
    }
    options = frida_portal_options_new();
    if (!NIL_P(kws)) {
        certificate = rb_hash_aref(kws, ID2SYM(rb_intern("certificate")));
        if (!NIL_P(certificate)) {
            if (!RB_TYPE_P(certificate, T_STRING)) {
                raise_argerror("certificate must be a string.");
                goto invalid_arg;
            }
            GTlsCertificate *gcertificate;
            if (!rbGObject_unmarshal_certificate(StringValueCStr(certificate), &gcertificate)) {
                raise_argerror("invalid certificate.");
                goto invalid_arg;
            }
            frida_portal_options_set_certificate(options, gcertificate);
            g_object_unref(gcertificate);
        }
        token = rb_hash_aref(kws, ID2SYM(rb_intern("token")));
        if (!NIL_P(token)) {
            if (!RB_TYPE_P(token, T_STRING)) {
                raise_argerror("token must be a string.");
                goto invalid_arg;
            }
            frida_portal_options_set_token(options, StringValueCStr(token));
        }
        acl = rb_hash_aref(kws, ID2SYM(rb_intern("acl")));
        if (!NIL_P(acl)) {
            if (!RB_TYPE_P(acl, T_ARRAY) || !array_all_type(acl, T_STRING)) {
                raise_argerror("acl must be an array of strings.");
                goto invalid_arg;
            }
            gchar **c_acl;
            gint acl_count;
            rbGObject_unmarshal_strv(acl, &c_acl, &acl_count);
            frida_portal_options_set_acl(options, c_acl, acl_count);
            g_strfreev(c_acl);
        }
    }
    join_portal_sync_proxy_args args = {
        .handle = d->handle,
        .address = StringValueCStr(address),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(FridaPortalMembership *handle, join_portal_sync, &args);
    g_object_unref(options);
    return (PortalMembership_from_FridaPortalMembership(handle));

gerror:
    g_clear_object(&options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
invalid_arg:
    g_object_unref(options);
    return (Qnil);
}

void	define_Session()
{
    cSession = rb_define_class_under(mCFrida, "Session", cGObject);

    rb_define_method(cSession, "inspect", Session_inspect, 0);
    rb_define_alias(cSession, "to_s", "inspect");

    rb_define_method(cSession, "pid", Session_pid, 0);
    rb_define_method(cSession, "is_detached", Session_is_detached, 0);
    rb_define_method(cSession, "detach", Session_detach, 0);
    rb_define_method(cSession, "resume", Session_resume, 0);
    rb_define_method(cSession, "enable_child_gating", Session_enable_child_gating, 0);
    rb_define_method(cSession, "disable_child_gating", Session_disable_child_gating, 0);
    rb_define_method(cSession, "create_script", Session_create_script, -1);
    rb_define_method(cSession, "create_script_from_bytes", Session_create_script_from_bytes, -1);
    rb_define_method(cSession, "compile_script", Session_compile_script, -1);
    rb_define_method(cSession, "snapshot_script", Session_snapshot_script, -1);
    rb_define_method(cSession, "setup_peer_connection", Session_setup_peer_connection, -1);
    rb_define_method(cSession, "join_portal", Session_join_portal, -1);
}
