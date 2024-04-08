#include "FileMonitor.h"

VALUE	FileMonitor_from_FridaFileMonitor(FridaFileMonitor *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cFileMonitor);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = frida_unref;
    return (self);
}

GVL_FREE_PROXY_FUNC(enable_sync, void *handle)
{
    GError *gerr = NULL;

    frida_file_monitor_enable_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #enable() -> nil
*/
static VALUE FileMonitor_enable(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, enable_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(disable_sync, void *handle)
{
    GError *gerr = NULL;

    frida_file_monitor_disable_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #disable() -> nil
*/
static VALUE FileMonitor_disable(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, disable_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

void	define_FileMonitor()
{
    cFileMonitor = rb_define_class_under(mCFrida, "FileMonitor", cGObject);
    rb_define_method(cFileMonitor, "enable", FileMonitor_enable, 0);
    rb_define_method(cFileMonitor, "disable", FileMonitor_disable, 0);
}
