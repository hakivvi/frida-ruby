#include "c_frida.h"

VALUE _gvl_bridge_thread;

VALUE mCFrida;
VALUE cGObject;
VALUE cDevice;
VALUE cDeviceManager;
VALUE cBus;
VALUE cIOStream;
VALUE cApplication;
VALUE cProcess;
VALUE cSpawn;
VALUE cChild;
VALUE cSession;
VALUE cScript;
VALUE cRelay;
VALUE cPortalMembership;
VALUE cCrash;
VALUE cFileMonitor;
VALUE cCompiler;
VALUE cEndpointParameters;
VALUE cPortalService;

void	raise_argerror(char *err)
{
    rb_raise(rb_eArgError, "%s", err);
}

void	raise_rerror(char *err, GError *gerr)
{
    if (!gerr)
        rb_raise(rb_eRuntimeError, "%s", err);
    else if (gerr->message) {
        VALUE ruby_string = rb_str_new_cstr(gerr->message);
        rb_funcall(ruby_string, rb_intern("force_encoding"), 1, rb_const_get(rb_cEncoding, rb_intern("UTF_8")));
        rb_funcall(rb_mKernel, rb_intern("raise"), 2, rb_eRuntimeError, ruby_string);
        g_error_free(gerr);
    } else
        raise_rerror("error.", NULL);
}

void Init_c_frida(void)
{
    rb_require("date");
    mCFrida = rb_define_module("CFrida");
    frida_init();
    rb_define_const(mCFrida, "FRIDA_VERSION", rb_str_new_cstr(frida_version_string()));
    define_GObject();
    define_Device();
    define_DeviceManager();
    define_Bus();
    define_IOStream();
    define_Application();
    define_Process();
    define_Spawn();
    define_Child();
    define_Session();
    define_Script();
    define_Relay();
    define_PortalMembership();
    define_Crash();
    define_FileMonitor();
    define_Compiler();
    define_EndpointParameters();
    define_PortalService();
    _gvl_bridge_thread = rb_thread_create(RUBY_METHOD_FUNC(gvl_bridge), NULL);
}
