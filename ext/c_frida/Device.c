#include "Device.h"

VALUE	Device_from_FridaDevice(FridaDevice *device)
{
    GVariant * icon;
    VALUE self;

    if (!device) return (Qnil);
    self = rb_class_new_instance(0, NULL, cDevice);
    GET_GOBJECT_DATA();
    d->handle = device;
    d->destroy = frida_unref;
    rb_ivar_set(self, rb_intern("id"), rb_str_new_cstr(frida_device_get_id(d->handle)));
    rb_ivar_set(self, rb_intern("name"), rb_str_new_cstr(frida_device_get_name(d->handle)));
    rb_ivar_set(self, rb_intern("type"), rbGObject_marshal_enum(frida_device_get_dtype(d->handle), FRIDA_TYPE_DEVICE_TYPE));
    rb_ivar_set(self, rb_intern("bus"), Bus_from_FridaBus(g_object_ref(frida_device_get_bus(d->handle))));
    icon = frida_device_get_icon(d->handle);
    rb_ivar_set(self, rb_intern("icon"), icon ? rbGObject_marshal_variant(icon) : Qnil);
    return (self);
}

/*
    call-seq:
        #id() -> String
*/
static VALUE Device_id(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("id")));
}

/*
    call-seq:
        #name() -> String
*/
static VALUE Device_name(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("name")));
}

/*
    call-seq:
        #type() -> String
*/
static VALUE Device_type(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("type")));
}

/*
    call-seq:
        #bus() -> Bus
*/
static VALUE Device_bus(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("bus")));
}

/*
    call-seq:
        #icon() -> String
*/
static VALUE Device_icon(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("icon")));
}

static VALUE Device_inspect(VALUE self)
{
    VALUE	s, id, name, type;

    id = rb_funcall(self, rb_intern("id"), 0, NULL);
    name = rb_funcall(self, rb_intern("name"), 0, NULL);
    type = rb_funcall(self, rb_intern("type"), 0, NULL);
    s = rb_sprintf("#<Device: id=%+"PRIsVALUE", name=%+"PRIsVALUE", type=%+"PRIsVALUE">", id, name, type);
    return (s);
}

GVL_FREE_PROXY_FUNC(open_channel_sync, open_channel_proxy_args *args)
{
    GError	*gerr = NULL;
    GIOStream *stream;

    stream = frida_device_open_channel_sync(args->device_handle, args->address, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(stream);
}

/*
    call-seq:
        #open_channel(address) -> IOStream
*/
static VALUE Device_open_channel(VALUE self, VALUE address)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    if (!RB_TYPE_P(address, T_STRING)) {
        raise_argerror("address should be a string.");
        return (Qnil);
    }
    open_channel_proxy_args args = {
        .device_handle = d->handle,
        .address = StringValueCStr(address)
    };
    CALL_GVL_FREE_WITH_RET(GIOStream *stream, open_channel_sync, &args);
    return (IOStream_from_GIOStream(stream));

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(is_lost, void *device_handle)
{
    GError	*gerr = NULL;
    void	*is_lost = NULL;

    is_lost += frida_device_is_lost(device_handle);
    RETURN_GVL_FREE_RESULT(is_lost);
}

/*
    call-seq:
        #is_lost() -> [TrueClass, FalseClass]
*/
static VALUE Device_is_lost(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    CALL_GVL_FREE_WITH_RET(void *is_lost, is_lost, d->handle);
    return (is_lost ? Qtrue : Qfalse);

    UNREACHABLE_GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(query_system_parameters_sync, void *device_handle)
{
    GError	*gerr = NULL;
    GHashTable	*params;

    params = frida_device_query_system_parameters_sync(device_handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(params);
}

/*
    call-seq:
        #query_system_parameters() -> Hash
*/
static VALUE Device_query_system_parameters(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    CALL_GVL_FREE_WITH_RET(void *params, query_system_parameters_sync, d->handle);
    return (rbGObject_marshal_dict(params));

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(get_frontmost_application_sync, get_frontmost_application_proxy_args *args)
{
    GError	*gerr = NULL;
    void	*app;

    app = frida_device_get_frontmost_application_sync(args->device_handle, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(app);
}

/*
    call-seq:
        #get_frontmost_application(scope:) -> Application
*/
static VALUE Device_get_frontmost_application(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaFrontmostQueryOptions *options;
    VALUE scope = Qnil;
    VALUE kws;

    rb_scan_args(argc, argv, ":", &kws);
    options = frida_frontmost_query_options_new();
    if (!NIL_P(kws)) {
        scope = rb_hash_aref(kws, ID2SYM(rb_intern("scope")));
        if (!NIL_P(scope)) {
            if (!RB_TYPE_P(scope, T_STRING)) {
                raise_argerror("scope must be a string.");
                return (Qnil);
            }
            FridaScope fscope;
            if (!rbGObject_unmarshal_enum(StringValueCStr(scope), FRIDA_TYPE_SCOPE, &fscope))
                goto invalid_argument;
            frida_frontmost_query_options_set_scope(options, scope);
        }
    }
    get_frontmost_application_proxy_args args = {
        .device_handle = d->handle,
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(void *app, get_frontmost_application_sync, &args);
    g_object_unref (options);
    return (Application_from_FridaApplication(app));

invalid_argument:
    g_object_unref(options);
    return (Qnil);
gerror:
    g_object_unref(options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

int array_all_type(VALUE arr, VALUE rtype)
{
    long len = RARRAY_LEN(arr);

    for (uint i = 0; i < len; i++) {
        if (!RB_TYPE_P(RARRAY_AREF(arr, i), rtype))
            return (0);
    }
    return (1);
}

GVL_FREE_PROXY_FUNC(enumerate_processes_sync, enumerate_processes_proxy_args *args)
{
    GError *gerr;
    void	*procs;

    procs = frida_device_enumerate_processes_sync(args->device_handle, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(procs);
}

/*
    call-seq:
        #enumerate_processes(pids:, scope:) -> Array[Process]
*/
static VALUE Device_enumerate_processes(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaProcessQueryOptions *options;
    VALUE pids, scope = Qnil;
    VALUE kws;

    rb_scan_args(argc, argv, ":", &kws);
    options = frida_process_query_options_new();
    if (!NIL_P(kws)) {
        pids = rb_hash_aref(kws, ID2SYM(rb_intern("pids")));
        if (!NIL_P(pids)) {
            if (!RB_TYPE_P(pids, T_ARRAY) || !array_all_type(pids, T_FIXNUM)) {
                raise_argerror("pids must be an array of integers.");
                return (Qnil);
            }
            long len = RARRAY_LEN(pids);
            VALUE pid;
            for (uint i = 0; i < len; i++) {
                pid = RARRAY_AREF(pids, i);
                frida_process_query_options_select_pid(options, NUM2UINT(pid));
            }
        }
        scope = rb_hash_aref(kws, ID2SYM(rb_intern("scope")));
        if (!NIL_P(scope)) {
            if (!RB_TYPE_P(scope, T_STRING)) {
                raise_argerror("scope must be a string.");
                return (Qnil);
            }
            FridaScope frida_scope;
            if (!rbGObject_unmarshal_enum(StringValueCStr(scope), FRIDA_TYPE_SCOPE, &frida_scope))
                goto error;
            frida_process_query_options_set_scope(options, frida_scope);
        }
    }
    enumerate_processes_proxy_args args = {
        .device_handle = d->handle,
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(FridaProcessList *procs, enumerate_processes_sync, &args);
    g_object_unref(options);
    int procs_count = frida_process_list_size(procs);
    VALUE procs_arr = rb_ary_new_capa(procs_count);
    for (gint i = 0; i < procs_count; i++) {
        rb_ary_store(procs_arr, i, Process_from_FridaProcess(frida_process_list_get(procs, i)));
    }
    g_object_unref(procs);
    return (procs_arr);

gerror:
    raise_rerror(NULL, _gerr);
error:
    g_object_unref(options);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(enumerate_applications_sync, enumerate_applications_proxy_args *args)
{
    GError *gerr;
    void	*apps;

    apps = frida_device_enumerate_applications_sync(args->device_handle, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(apps);
}

/*
    call-seq:
        #enumerate_applications(identifiers:, scope:) -> Array[Application]
*/
static VALUE Device_enumerate_applications(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaApplicationQueryOptions *options;
    VALUE identifiers, scope = Qnil;
    VALUE kws;

    rb_scan_args(argc, argv, ":", &kws);
    options = frida_application_query_options_new();
    if (!NIL_P(kws)) {
        identifiers = rb_hash_aref(kws, ID2SYM(rb_intern("identifiers")));
        if (!NIL_P(identifiers)) {
            if (!RB_TYPE_P(identifiers, T_ARRAY) || !array_all_type(identifiers, T_STRING)) {
                raise_argerror("identifiers must be an array of strings.");
                return (Qnil);
            }
            long len = RARRAY_LEN(identifiers);
            VALUE id;
            for (uint i = 0; i < len; i++) {
                id = RARRAY_AREF(identifiers, i);
                frida_application_query_options_select_identifier(options, StringValueCStr(id));
            }
        }
        scope = rb_hash_aref(kws, ID2SYM(rb_intern("scope")));
        if (!NIL_P(scope)) {
            if (!RB_TYPE_P(scope, T_STRING)) {
                raise_argerror("scope must be a string.");
                return (Qnil);
            }
            FridaScope frida_scope;
            if (!rbGObject_unmarshal_enum(StringValueCStr(scope), FRIDA_TYPE_SCOPE, &frida_scope))
                goto error;
            frida_application_query_options_set_scope(options, frida_scope);
        }
    }
    enumerate_applications_proxy_args args = {
        .device_handle = d->handle,
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(FridaApplicationList *apps, enumerate_applications_sync, &args);
    g_object_unref(options);
    int apps_count = frida_application_list_size(apps);
    VALUE apps_arr = rb_ary_new_capa(apps_count);
    for (gint i = 0; i < apps_count; i++) {
        rb_ary_store(apps_arr, i, Application_from_FridaApplication(frida_application_list_get(apps, i)));
    }
    g_object_unref(apps);
    return (apps_arr);

gerror:
    raise_rerror(NULL, _gerr);
error:
    g_object_unref(options);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(enable_spawn_gating_sync, void *device_handle)
{
    GError *gerr = NULL;

    frida_device_enable_spawn_gating_sync(device_handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #enable_spawn_gating() -> nil
*/
static VALUE Device_enable_spawn_gating(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, enable_spawn_gating_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(disable_spawn_gating_sync, void *device_handle)
{
    GError *gerr = NULL;

    frida_device_disable_spawn_gating_sync(device_handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #disable_spawn_gating() -> nil
*/
static VALUE Device_disable_spawn_gating(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, disable_spawn_gating_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

static int hash_all_type(VALUE hash, VALUE rtype)
{
    VALUE keys = rb_funcall(hash, rb_intern("keys"), 0);

    long keys_count = RARRAY_LEN(keys);
    for (uint i = 0; i < keys_count; i++) {
        VALUE key = rb_ary_entry(keys, i);
        VALUE value = rb_hash_aref(hash, key);
        if (!RB_TYPE_P(key, rtype) || !RB_TYPE_P(value, rtype)) {
            return 0;
        }
    }
    return 1;
}

GVL_FREE_PROXY_FUNC(spawn_sync, spawn_sync_proxy_args *args)
{
    GError *gerr;
    char    *pid = NULL;

    pid += frida_device_spawn_sync(args->device_handle, args->program, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(pid);
}

static FridaSpawnOptions *parse_spwan_options(FridaSpawnOptions * options, VALUE kws)
{
    VALUE argv, envp, env, cwd, rstdio, aux;

    argv = rb_hash_aref(kws, ID2SYM(rb_intern("argv")));
    if (!NIL_P(argv)) {
        if (!RB_TYPE_P(argv, T_ARRAY) || !array_all_type(argv, T_STRING)) {
            raise_argerror("argv must be an array of strings.");
            return (NULL);
        }
        gchar **c_argv;
        gint argv_count;
        rbGObject_unmarshal_strv(argv, &c_argv, &argv_count);
        frida_spawn_options_set_argv(options, c_argv, argv_count);
        g_strfreev(c_argv);
    }
    envp = rb_hash_aref(kws, ID2SYM(rb_intern("envp")));
    if (!NIL_P(envp)) {
        if (!RB_TYPE_P(envp, T_HASH) || !hash_all_type(envp, T_STRING)) {
            raise_argerror("envp must be a hash of keys and values as strings.");
            return (NULL);
        }
        gchar **c_envp;
        gint envp_count;
        rbGObject_unmarshal_envp(envp, &c_envp, &envp_count);
        frida_spawn_options_set_envp(options, c_envp, envp_count);
    }
    env = rb_hash_aref(kws, ID2SYM(rb_intern("env")));
    if (!NIL_P(env)) {
        if (!RB_TYPE_P(env, T_HASH) || !hash_all_type(env, T_STRING)) {
            raise_argerror("env must be a hash of keys and values as strings.");
            return (NULL);
        }
        gchar **c_env;
        gint env_count;
        rbGObject_unmarshal_envp(env, &c_env, &env_count);
        frida_spawn_options_set_env(options, c_env, env_count);
    }
    cwd = rb_hash_aref(kws, ID2SYM(rb_intern("cwd")));
    if (!NIL_P(cwd)) {
        if (!RB_TYPE_P(cwd, T_STRING)) {
            raise_argerror("cwd must be a string.");
            return (NULL);
        }
        frida_spawn_options_set_cwd(options, StringValueCStr(cwd));
    }
    rstdio = rb_hash_aref(kws, ID2SYM(rb_intern("stdio")));
    if (!NIL_P(rstdio)) {
        if (!RB_TYPE_P(rstdio, T_STRING)) {
            raise_argerror("stdio must be a string.");
            return (NULL);
        }
        FridaStdio stdio;
        if (!rbGObject_unmarshal_enum(StringValueCStr(rstdio), FRIDA_TYPE_STDIO, &stdio)) {
            raise_argerror("stdio is invalid.");
            return (NULL);
        }
        frida_spawn_options_set_stdio(options, stdio);
    }
    aux = rb_hash_aref(kws, ID2SYM(rb_intern("aux")));
    if (!NIL_P(aux)) {
        if (!RB_TYPE_P(aux, T_HASH)) {
            raise_argerror("aux must be a hash.");
            return (NULL);
        }
        VALUE keys = rb_funcall(aux, rb_intern("keys"), 0);
        long keys_count = RARRAY_LEN(keys);
        GHashTable *gaux = frida_spawn_options_get_aux(options);
        GVariant *gvalue;
        gchar *gkey;
        for (uint i = 0; i < keys_count; i++) {
            VALUE key = rb_ary_entry(keys, i);
            if (!RB_TYPE_P(key, T_STRING)) {
                raise_argerror("aux hash keys must be strings.");
                return (NULL);
            }
            rbGObject_unmarshal_string(key, &gkey);
            VALUE value = rb_hash_aref(aux, key);
            if (!rbGObject_unmarshal_variant(value, &gvalue))
                return (g_free(gkey), NULL);
            g_hash_table_insert(gaux, gkey,	g_variant_ref_sink(gvalue));
        }
    }
    return (options);
}

/*
    call-seq:
        #spawn(program, argv:, envp:, env:, cwd:, stdio:, aux:) -> nil
*/
static VALUE Device_spawn(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE program, kws;

    FridaSpawnOptions *options = frida_spawn_options_new();
    rb_scan_args(argc, argv, "1:", &program, &kws);
    if (!RB_TYPE_P(program, T_STRING)) {
        raise_argerror("program must be a string.");
        return (Qnil);
    }
    if (!NIL_P(kws)) {
        options = parse_spwan_options(options, kws);
        if (!options)
            return (g_object_unref(options), Qnil);
    }
    spawn_sync_proxy_args args = {
        .device_handle = d->handle,
        .program = StringValueCStr(program),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(void *pid, spawn_sync, &args);
    g_object_unref(options);
    return (UINT2NUM((unsigned long long)pid / sizeof(char)));

gerror:
    g_object_unref(options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(resume_sync, resume_sync_proxy_args *args)
{
    GError *gerr = NULL;

    frida_device_resume_sync(args->device_handle, args->pid, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #resume(pid) -> nil
*/
static VALUE Device_resume(VALUE self, VALUE pid)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(pid, T_FIXNUM)) {
        raise_argerror("pid must be a number.");
        return (Qnil);
    }
    resume_sync_proxy_args args = {
        .device_handle = d->handle,
        .pid = NUM2UINT(pid)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, resume_sync, &args);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(input_sync, input_sync_proxy_args *args)
{
    GError *gerr = NULL;

    frida_device_input_sync(args->device_handle, args->pid, args->data, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #input(pid, buffer) -> nil
*/
static VALUE Device_input(VALUE self, VALUE pid, VALUE buffer)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(pid, T_FIXNUM)) {
        raise_argerror("pid must be a number.");
        return (Qnil);
    }
    if (!RB_TYPE_P(buffer, T_STRING)) {
        raise_argerror("buffer must be a string.");
        return (Qnil);
    }
    GBytes *data;
    data = g_bytes_new(RSTRING_PTR(buffer), RSTRING_LEN(buffer));
    input_sync_proxy_args args = {
        .device_handle = d->handle,
        .pid = NUM2UINT(pid),
        .data = data
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, input_sync, &args);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    g_bytes_unref(data);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(kill_sync, kill_sync_proxy_args *args)
{
    GError *gerr = NULL;

    frida_device_kill_sync(args->device_handle, args->pid, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #kill(pid) -> nil
*/
static VALUE Device_kill(VALUE self, VALUE pid)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(pid, T_FIXNUM)) {
        raise_argerror("pid must be a number.");
        return (Qnil);
    }
    kill_sync_proxy_args args = {
        .device_handle = d->handle,
        .pid = NUM2UINT(pid)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, kill_sync, &args);
    goto done;
gerror:
    raise_rerror(NULL, _gerr);
done:
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(unpair_sync, void *device_handle)
{
    GError *gerr = NULL;

    frida_device_unpair_sync(device_handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #unpair() -> nil
*/
static VALUE Device_unpair(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, unpair_sync, d->handle);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(inject_library_file_sync, inject_library_file_sync_proxy_args *args)
{
    GError *gerr = NULL;
    char	*id = NULL;

    id += frida_device_inject_library_file_sync(args->device_handle, args->pid, args->path, args->entrypoint, args->data, \
            NULL, &gerr);
    RETURN_GVL_FREE_RESULT(id);
}

/*
    call-seq:
        #inject_library_file(pid, path, entrypoint, data) -> Fixnum
*/
static VALUE Device_inject_library_file(VALUE self, VALUE pid, VALUE path, VALUE entrypoint, VALUE data)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(pid, T_FIXNUM)) {
        raise_argerror("pid must be a number.");
        return (Qnil);
    }
    if (!RB_TYPE_P(path, T_STRING)) {
        raise_argerror("path must be a string.");
        return (Qnil);
    }
    if (!RB_TYPE_P(entrypoint, T_STRING)) {
        raise_argerror("entrypoint must be a string.");
        return (Qnil);
    }
    if (!RB_TYPE_P(data, T_STRING)) {
        raise_argerror("data must be a string.");
        return (Qnil);
    }
    inject_library_file_sync_proxy_args args = {
        .device_handle = d->handle,
        .pid = NUM2UINT(pid),
        .path = StringValueCStr(path),
        .entrypoint = StringValueCStr(entrypoint),
        .data = StringValueCStr(data)
    };
    CALL_GVL_FREE_WITH_RET(void *id, inject_library_file_sync, &args);
    return (UINT2NUM((unsigned long long)id / sizeof(char)));

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(inject_library_blob_sync, inject_library_blob_sync_proxy_args *args)
{
    GError *gerr = NULL;
    char	*id = NULL;

    id += frida_device_inject_library_blob_sync(args->device_handle, args->pid, args->blob, args->entrypoint, args->data, \
            NULL, &gerr);
    RETURN_GVL_FREE_RESULT(id);
}

/*
    call-seq:
        #inject_library_blob(pid, blob, entrypoint, data) -> Fixnum
*/
static VALUE Device_inject_library_blob(VALUE self, VALUE pid, VALUE blob, VALUE entrypoint, VALUE data)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    GBytes *gblob;

    if (!RB_TYPE_P(pid, T_FIXNUM)) {
        raise_argerror("pid must be a number.");
        return (Qnil);
    }
    if (!RB_TYPE_P(blob, T_STRING)) {
        raise_argerror("blob must be a string.");
        return (Qnil);
    }
    if (!RB_TYPE_P(entrypoint, T_STRING)) {
        raise_argerror("entrypoint must be a string.");
        return (Qnil);
    }
    if (!RB_TYPE_P(data, T_STRING)) {
        raise_argerror("data must be a string.");
        return (Qnil);
    }
    gblob = g_bytes_new(RSTRING_PTR(blob), RSTRING_LEN(blob));
    inject_library_blob_sync_proxy_args args = {
        .device_handle = d->handle,
        .pid = NUM2UINT(pid),
        .blob = gblob,
        .entrypoint = StringValueCStr(entrypoint),
        .data = StringValueCStr(data)
    };
    CALL_GVL_FREE_WITH_RET(void *id, inject_library_file_sync, &args);
    g_bytes_unref(gblob);
    return (UINT2NUM((unsigned long long)id / sizeof(char)));

gerror:
    g_bytes_unref(gblob);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(enumerate_pending_spawn_sync, void *device_handle)
{
    GError *gerr = NULL;
    FridaSpawnList *spawn_list;

    spawn_list = frida_device_enumerate_pending_spawn_sync(device_handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(spawn_list);
}

/*
    call-seq:
        #enumerate_pending_spawn() -> Array
*/
static VALUE Device_enumerate_pending_spawn(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    guint spawn_list_size;
    VALUE spawn_list_array;

    CALL_GVL_FREE_WITH_RET(FridaSpawnList *spawn_list, enumerate_pending_spawn_sync, d->handle);
    spawn_list_size = frida_spawn_list_size(spawn_list);
    spawn_list_array = rb_ary_new_capa(spawn_list_size);
    for (uint i = 0; i < spawn_list_size; i++) {
        rb_ary_store(spawn_list_array, i, Spawn_from_FridaSpawn(frida_spawn_list_get(spawn_list, i)));
    }
    g_object_unref(spawn_list);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
    return (Qnil);
done:
    return (spawn_list_array);
}

GVL_FREE_PROXY_FUNC(enumerate_pending_children_sync, void *device_handle)
{
    GError *gerr = NULL;
    FridaChildList *child_list;

    child_list = frida_device_enumerate_pending_children_sync(device_handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(child_list);
}

/*
    call-seq:
        #enumerate_pending_children() -> Array
*/
static VALUE Device_enumerate_pending_children(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    guint child_list_size;
    VALUE child_list_array;

    CALL_GVL_FREE_WITH_RET(FridaChildList *child_list, enumerate_pending_children_sync, d->handle);
    child_list_size = frida_child_list_size(child_list);
    child_list_array = rb_ary_new_capa(child_list_size);
    for (uint i = 0; i < child_list_size; i++) {
        rb_ary_store(child_list_array, i, Child_from_FridaChild(frida_child_list_get(child_list, i)));
    }
    g_object_unref(child_list);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
    return (Qnil);
done:
    return (child_list_array);
}

GVL_FREE_PROXY_FUNC(attach_sync, attach_sync_proxy_args *args)
{
    GError *gerr = NULL;
    FridaSession *session;

    session = frida_device_attach_sync(args->device_handle, args->pid, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(session);
}

/*
    call-seq:
        #attach(pid, realm:, persist_timeout:) -> Session
*/
static VALUE Device_attach(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE pid, kws;
    FridaSessionOptions *options;

    rb_scan_args(argc, argv, "1:", &pid, &kws);
    options = frida_session_options_new();
    if (!RB_TYPE_P(pid, T_FIXNUM)) {
        raise_argerror("pid must be a number.");
        return (Qnil);
    }
    if (!NIL_P(kws)) {
        VALUE realm;
        realm = rb_hash_aref(kws, ID2SYM(rb_intern("realm")));
        if (!NIL_P(realm)) {
            if (!RB_TYPE_P(realm, T_STRING)) {
                raise_argerror("realm must be a string.");
                return (Qnil);
            }
            FridaRealm grealm;
            if (!rbGObject_unmarshal_enum(StringValueCStr(realm), FRIDA_TYPE_REALM, &grealm)) {
                g_object_unref(options);
                raise_argerror("invalid realm.");
                return (Qnil);
            }
            frida_session_options_set_realm(options, grealm);
        }

        VALUE persist_timeout;
        persist_timeout = rb_hash_aref(kws, ID2SYM(rb_intern("persist_timeout")));
        if (!NIL_P(persist_timeout)) {
            if (!RB_TYPE_P(persist_timeout, T_STRING)) {
                raise_argerror("persist_timeout must be a number.");
                return (Qnil);
            }
        }
        frida_session_options_set_persist_timeout(options, NUM2UINT(persist_timeout));
    }
    attach_sync_proxy_args args = {
        .device_handle = d->handle,
        .pid = NUM2UINT(pid),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(FridaSession *session, attach_sync, &args);
    g_object_unref(options);
    return (Session_from_FridaSession(session));

gerror:
    g_object_unref(options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

void	define_Device()
{
    cDevice = rb_define_class_under(mCFrida, "Device", cGObject);

    rb_define_method(cDevice, "inspect", Device_inspect, 0);
    rb_define_alias(cDevice, "to_s", "inspect");
    rb_define_method(cDevice, "id", Device_id, 0);
    rb_define_method(cDevice, "name", Device_name, 0);
    rb_define_method(cDevice, "type", Device_type, 0);
    rb_define_method(cDevice, "bus", Device_bus, 0);
    rb_define_method(cDevice, "icon", Device_icon, 0);

    rb_define_method(cDevice, "open_channel", Device_open_channel, 1);
    rb_define_method(cDevice, "is_lost", Device_is_lost, 0);
    rb_define_method(cDevice, "query_system_parameters", Device_query_system_parameters, 0);
    rb_define_method(cDevice, "get_frontmost_application", Device_get_frontmost_application, -1);
    rb_define_method(cDevice, "enumerate_applications", Device_enumerate_applications, -1);
    rb_define_method(cDevice, "enumerate_processes", Device_enumerate_processes, -1);
    rb_define_method(cDevice, "enable_spawn_gating", Device_enable_spawn_gating, 0);
    rb_define_method(cDevice, "disable_spawn_gating", Device_disable_spawn_gating, 0);
    rb_define_method(cDevice, "spawn", Device_spawn, -1);
    rb_define_method(cDevice, "resume", Device_resume, 1);
    rb_define_method(cDevice, "input", Device_input, 2);
    rb_define_method(cDevice, "kill", Device_kill, 1);
    rb_define_method(cDevice, "unpair", Device_unpair, 0);
    rb_define_method(cDevice, "inject_library_file", Device_inject_library_file, 4);
    rb_define_method(cDevice, "inject_library_blob", Device_inject_library_blob, 4);
    rb_define_method(cDevice, "enumerate_pending_spawn", Device_enumerate_pending_spawn, 0);
    rb_define_method(cDevice, "enumerate_pending_children", Device_enumerate_pending_children, 0);
    rb_define_method(cDevice, "attach", Device_attach, -1);
}
