#include "PortalMembership.h"

VALUE	PortalMembership_from_FridaPortalMembership(FridaPortalMembership *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cPortalMembership);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = frida_unref;
    return (self);
}

GVL_FREE_PROXY_FUNC(terminate_sync, FridaPortalMembership *handle)
{
    GError *gerr = NULL;

    frida_portal_membership_terminate_sync(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #terminate() -> nil
*/
static VALUE PortalMembership_terminate(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    CALL_GVL_FREE_WITH_RET(void *dummy, terminate_sync, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

void	define_PortalMembership()
{
    cPortalMembership = rb_define_class_under(mCFrida, "PortalMembership", cGObject);

    rb_define_method(cPortalMembership, "terminate", PortalMembership_terminate, 0);
}
