#include "DeviceManager.h"

VALUE DeviceManager_from_FridaDeviceManager(FridaDeviceManager *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cDeviceManager);
    GET_GOBJECT_DATA();
    d->destroy(d->handle);
    d->handle = handle;
    return (self);
}

GVL_FREE_PROXY_FUNC(close_device_manager_err, FridaDeviceManager *device_manager)
{
    GError *gerr = NULL;

    frida_device_manager_close_sync(device_manager, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

GVL_FREE_PROXY_FUNC(close_device_manager, FridaDeviceManager *device_manager)
{
    frida_device_manager_close_sync(device_manager, NULL, NULL);
    return (NULL);
}

static void DeviceManager_destroy(void *handle)
{
    CALL_GVL_FREE(close_device_manager, handle);
    frida_unref(handle);
}

/*
    call-seq:
        #close() -> nil
*/
static VALUE DeviceManager_close(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    CALL_GVL_FREE_WITH_RET(void *dummy, close_device_manager_err, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(enumerate_devices_sync, FridaDeviceManager *device_manager)
{
    GError *gerr = NULL;
    FridaDeviceList * devices;

    devices = frida_device_manager_enumerate_devices_sync(device_manager, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(devices);
}

/*
    call-seq:
        #enumerate_device() -> Array[Device]
*/
static VALUE DeviceManager_enumerate_devices(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    FridaDeviceList * devices;
    int devices_count;
    VALUE Devices_arr;

    CALL_GVL_FREE_WITH_RET(devices, enumerate_devices_sync, d->handle);
    devices_count = frida_device_list_size(devices);
    Devices_arr = rb_ary_new_capa(devices_count);
    for (int i = 0; i < devices_count; i++)
        rb_ary_store(Devices_arr, i, Device_from_FridaDevice(frida_device_list_get(devices, i)));
    frida_unref(devices);
    return (Devices_arr);

    GERROR_BLOCK
}

/*
    call-seq:
        #get_device_matching() {} -> Device
*/
static VALUE DeviceManager_get_device_matching(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE block;
    VALUE devices, current_device, block_ret;
    int	devices_count;

    if (!rb_block_given_p()) {
        raise_argerror("a block argument is required");
        return (Qnil);
    }
    block = rb_block_proc();
    // NOTE: Ruby does not support (currently) setting up a non Ruby created thread to be able
    // to call its C APIs, frida_device_manager_get_device_sync() would call our filtering predicate from a frida thread
    // which is not created by Ruby, and GET_EC() call in the Ruby's C APIs would return NULL.
    // so the filtering is done in this thread that currently holds the GVL, mimicking frida_device_manager_get_device_sync().
    devices = rb_funcall(self, rb_intern("enumerate_devices"), 0, NULL);
    devices_count = RARRAY_LEN(devices);
    for (int i = 0; i < devices_count; i++) {
        current_device = rb_ary_entry(devices, i);
        block_ret = rb_funcall(block, rb_intern("call"), 1, current_device);
        if (block_ret == Qtrue) return (current_device);
    }
    return (Qnil);
}

static FridaRemoteDeviceOptions *parse_add_rdevice_kws_to_options(VALUE kws)
{
    FridaRemoteDeviceOptions *options;
    VALUE cert, origin, token, keepalive_interval;

    options = frida_remote_device_options_new();
    cert = rb_hash_aref(kws, ID2SYM(rb_intern("certificate")));
    if (!NIL_P(cert)) {
        if (!RB_TYPE_P(cert, T_STRING)) {
            raise_argerror("certificate should be a string.");
            goto error;
        }
        GTlsCertificate * certificate;
        if (!rbGObject_unmarshal_certificate(StringValueCStr(cert), &certificate))
            goto error;
        frida_remote_device_options_set_certificate(options, certificate);
        g_object_unref(certificate);
    }
    origin = rb_hash_aref(kws, ID2SYM(rb_intern("origin")));
    if (!NIL_P(origin)) {
        if (!RB_TYPE_P(origin, T_STRING)) {
            raise_argerror("origin should be a string.");
            goto error;
        }
        frida_remote_device_options_set_origin(options, StringValueCStr(origin));
    }
    token = rb_hash_aref(kws, ID2SYM(rb_intern("token")));
    if (!NIL_P(token)) {
        if (!RB_TYPE_P(token, T_STRING)) {
            raise_argerror("token should be a string.");
            goto error;
        }
        frida_remote_device_options_set_token(options, StringValueCStr(token));
    }
    keepalive_interval = rb_hash_aref(kws, ID2SYM(rb_intern("keepalive_interval")));
    if (!NIL_P(keepalive_interval)) {
        if (!RB_TYPE_P(keepalive_interval, T_FIXNUM)) {
            raise_argerror("keepalive_interval should be a number.");
            goto error;
        }
        frida_remote_device_options_set_keepalive_interval(options, NUM2INT(keepalive_interval));
    }
    return (options);

error:
    g_object_unref(options);
    return (NULL);
}

GVL_FREE_PROXY_FUNC(add_remote_device_sync, add_remote_device_proxy_args *args)
{
    GError *gerr = NULL;
    FridaDevice	*device;

    device = frida_device_manager_add_remote_device_sync(args->device_manager, args->address, args->options, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(device);
}

/*
    call-seq:
        #add_remote_device(address, certificate:, origin:, token:, keepalive_interval:) -> Device
*/
static VALUE DeviceManager_add_remote_device(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    VALUE	address, kws;
    FridaRemoteDeviceOptions *options;
    FridaDevice *device;

    rb_scan_args(argc, argv, "1:", &address, &kws);
    if (!RB_TYPE_P(address, T_STRING))
        return (raise_argerror("address must be a string."), Qnil);
    if (NIL_P(kws)) kws = rb_hash_new();
    options = parse_add_rdevice_kws_to_options(kws);
    if (!options)
        return (Qnil);
    add_remote_device_proxy_args args = {
        .device_manager = d->handle,
        .options = options,
        .address = StringValueCStr(address)
    };
    CALL_GVL_FREE_WITH_RET(device, add_remote_device_sync, &args);
    g_object_unref(options);
    return (Device_from_FridaDevice(device));

gerror:
    g_object_unref(options);
    raise_rerror(NULL, _gerr);
    return (Qnil);
}

GVL_FREE_PROXY_FUNC(remove_remote_device_sync, remove_remote_devices_proxy_args *args)
{
    GError	*gerr = NULL;

    frida_device_manager_remove_remote_device_sync(args->device_manager, args->address, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #remove_remote_device(address) -> nil
*/
static VALUE DeviceManager_remove_remote_device(VALUE self, VALUE address)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    if (!RB_TYPE_P(address, T_STRING)) {
        raise_argerror("address should be a string.");
        return (Qnil);
    }
    remove_remote_devices_proxy_args args = {
        .device_manager = d->handle,
        .address = StringValueCStr(address)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, remove_remote_device_sync, &args);
    return (Qnil);

    GERROR_BLOCK
}

/*
    call-seq:
        #new() -> DeviceManager
*/
static VALUE DeviceManager_initialize(VALUE self)
{
    GET_GOBJECT_DATA();
    d->handle = frida_device_manager_new();
    if (!d->handle)
        return (raise_rerror("failed to retrieve a device manager.", NULL), Qnil);
    d->destroy = DeviceManager_destroy;
    return (self);
}

void	define_DeviceManager()
{
    cDeviceManager = rb_define_class_under(mCFrida, "DeviceManager", cGObject);

    rb_define_method(cDeviceManager, "initialize", DeviceManager_initialize, 0);
    rb_define_method(cDeviceManager, "enumerate_devices", DeviceManager_enumerate_devices, 0);
    rb_define_method(cDeviceManager, "get_device_matching", DeviceManager_get_device_matching, 0);
    rb_define_method(cDeviceManager, "add_remote_device", DeviceManager_add_remote_device, -1);
    rb_define_method(cDeviceManager, "remove_remote_device", DeviceManager_remove_remote_device, 1);
    rb_define_method(cDeviceManager, "close", DeviceManager_close, 0);
}
