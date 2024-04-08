#include "Compiler.h"

VALUE	Compiler_from_FridaCompiler(FridaCompiler *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    VALUE args[1] = {Qfalse};
    self = rb_class_new_instance(1, args, cCompiler);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = frida_unref;
    return (self);
}

GVL_FREE_PROXY_FUNC(build_sync, build_sync_proxy_args *args)
{
    GError *gerr = NULL;
    char	*bundle;

    bundle = frida_compiler_build_sync(args->handle, args->entrypoint, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(bundle);
}

/*
    call-seq:
        #build(entrypoint, project_root:, source_maps:, compression:) -> String
*/
static VALUE Compiler_build(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    VALUE entrypoint, kws;
    FridaBuildOptions *options;
    FridaCompilerOptions *coptions;

    rb_scan_args(argc, argv, "1:", &entrypoint, &kws);
    if (!RB_TYPE_P(entrypoint, T_STRING)) {
        raise_argerror("entrypoint must be a string.");
        return (Qnil);
    }
    options = frida_build_options_new();
    coptions = FRIDA_COMPILER_OPTIONS(options);
    if (!NIL_P(kws)) {
        VALUE project_root = rb_hash_aref(kws, ID2SYM(rb_intern("project_root")));
        if (!NIL_P(project_root)) {
            if (!RB_TYPE_P(project_root, T_STRING)) {
                raise_argerror("project_root must be a string.");
                return (Qnil);
            }
            frida_compiler_options_set_project_root(coptions, StringValueCStr(project_root));
        }
        VALUE source_maps = rb_hash_aref(kws, ID2SYM(rb_intern("source_maps")));
        if (!NIL_P(source_maps)) {
            if (!RB_TYPE_P(source_maps, T_STRING)) {
                raise_argerror("source_maps must be a string.");
                return (Qnil);
            }
            FridaSourceMaps gsource_maps;
            if (!rbGObject_unmarshal_enum(StringValueCStr(source_maps), FRIDA_TYPE_SOURCE_MAPS, &gsource_maps)) {
                raise_argerror("invalid source_maps.");
                return (Qnil);
            }
            frida_compiler_options_set_source_maps(coptions, gsource_maps);
        }
        VALUE compression = rb_hash_aref(kws, ID2SYM(rb_intern("compression")));
        if (!NIL_P(compression)) {
            if (!RB_TYPE_P(compression, T_STRING)) {
                raise_argerror("compression must be a string.");
                return (Qnil);
            }
            FridaJsCompression gcompression;
            if (!rbGObject_unmarshal_enum(StringValueCStr(compression), FRIDA_TYPE_JS_COMPRESSION, &gcompression)) {
                raise_argerror("invalid compression.");
                return (Qnil);
            }
            frida_compiler_options_set_compression(coptions, gcompression);
        }
    }
    build_sync_proxy_args args = {
        .handle = d->handle,
        .entrypoint = StringValueCStr(entrypoint),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(char *bundle, build_sync, &args);
    g_object_unref(options);
    VALUE rbundle = rb_str_new_cstr(bundle);
    g_free(bundle);
    return (rbundle);

gerror:
    g_object_unref(options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(watch_sync, watch_sync_proxy_args *args)
{
    GError *gerr = NULL;

    frida_compiler_watch_sync(args->handle, args->entrypoint, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #watch(entrypoint, project_root:, source_maps:, compression:) -> nil
*/
static VALUE Compiler_watch(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    VALUE entrypoint, kws;
    FridaWatchOptions *options;
    FridaCompilerOptions *coptions;

    rb_scan_args(argc, argv, "1:", &entrypoint, &kws);
    if (!RB_TYPE_P(entrypoint, T_STRING)) {
        raise_argerror("entrypoint must be a string.");
        return (Qnil);
    }
    options = frida_watch_options_new();
    coptions = FRIDA_COMPILER_OPTIONS(options);
    if (!NIL_P(kws)) {
        VALUE project_root = rb_hash_aref(kws, ID2SYM(rb_intern("project_root")));
        if (!NIL_P(project_root)) {
            if (!RB_TYPE_P(project_root, T_STRING)) {
                raise_argerror("project_root must be a string.");
                return (Qnil);
            }
            frida_compiler_options_set_project_root(coptions, StringValueCStr(project_root));
        }
        VALUE source_maps = rb_hash_aref(kws, ID2SYM(rb_intern("source_maps")));
        if (!NIL_P(source_maps)) {
            if (!RB_TYPE_P(source_maps, T_STRING)) {
                raise_argerror("source_maps must be a string.");
                return (Qnil);
            }
            FridaSourceMaps gsource_maps;
            if (!rbGObject_unmarshal_enum(StringValueCStr(source_maps), FRIDA_TYPE_SOURCE_MAPS, &gsource_maps)) {
                raise_argerror("invalid source_maps.");
                return (Qnil);
            }
            frida_compiler_options_set_source_maps(coptions, gsource_maps);
        }
        VALUE compression = rb_hash_aref(kws, ID2SYM(rb_intern("compression")));
        if (!NIL_P(compression)) {
            if (!RB_TYPE_P(compression, T_STRING)) {
                raise_argerror("compression must be a string.");
                return (Qnil);
            }
            FridaJsCompression gcompression;
            if (!rbGObject_unmarshal_enum(StringValueCStr(compression), FRIDA_TYPE_JS_COMPRESSION, &gcompression)) {
                raise_argerror("invalid compression.");
                return (Qnil);
            }
            frida_compiler_options_set_compression(coptions, gcompression);
        }
    }
    watch_sync_proxy_args args = {
        .handle = d->handle,
        .entrypoint = StringValueCStr(entrypoint),
        .options = options
    };
    CALL_GVL_FREE_WITH_RET(char *dummy, watch_sync, &args);
    goto done;

gerror:
    raise_rerror(NULL, _gerr);
done:
    g_object_unref(options);
    return (Qnil);
}

/*
    call-seq:
        #new(dvmgr) -> Compiler
*/
static VALUE Compiler_initialize(VALUE self, VALUE dvmgr)
{
    GET_GOBJECT_DATA();

    if (dvmgr == Qfalse)
        return (self);
    if (!rb_obj_is_instance_of(dvmgr, cDeviceManager)) {
        raise_argerror("argument should be a DeviceManager instance.");
        return (Qnil);
    }
    VALUE dvmgr_self;
    dvmgr_self = dvmgr;
    GET_DATA_EX(GObject, dvmgr_self, dvmgr_d);
    d->handle = frida_compiler_new(dvmgr_d->handle);
    d->destroy = frida_unref;
    return (self);
}

void	define_Compiler()
{
    cCompiler = rb_define_class_under(mCFrida, "Compiler", cGObject);

    rb_define_method(cCompiler, "initialize", Compiler_initialize, 1);

    rb_define_method(cCompiler, "build", Compiler_build, -1);
    rb_define_method(cCompiler, "watch", Compiler_watch, -1);
}
