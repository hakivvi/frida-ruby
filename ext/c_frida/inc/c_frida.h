#ifndef FRIDA_RUBY_H
#define FRIDA_RUBY_H 1

#include "ruby.h"
#include "ruby/thread.h"
#include "ruby/vm.h"
#include "frida-core.h"

typedef struct _GObject_d {
    void	*handle;
    void	(*destroy)(void *);
    GSList	*signal_closures;
} GObject_d;

#define FRIDA_TYPE_RB_AUTHENTICATION_SERVICE frida_rb_authentication_service_get_type()
G_DECLARE_FINAL_TYPE(FridaRBAuthenticationService, frida_rb_authentication_service, FRIDA, RB_AUTHENTICATION_SERVICE, GObject)

#include "gutils.h"

#include "GObject.h"
#include "DeviceManager.h"
#include "Device.h"
#include "Bus.h"
#include "IOStream.h"
#include "Application.h"
#include "Process.h"
#include "Spawn.h"
#include "Child.h"
#include "Session.h"
#include "Script.h"
#include "Relay.h"
#include "PortalMembership.h"
#include "Crash.h"
#include "FileMonitor.h"
#include "Compiler.h"
#include "EndpointParameters.h"
#include "PortalService.h"

#include "gvl_bridge.h"

extern const rb_data_type_t GObject_dt;

#define DEFINE_KLASS_DATA_TYPE(klass) \
	const rb_data_type_t klass##_dt = { \
		.wrap_struct_name = "Frida::" #klass "Data", \
		.function = { \
			.dcompact = NULL, \
			.dmark = NULL, \
			.dsize = NULL, \
			.dfree = (void (*)(void*))klass##_free, \
		}, \
		.flags = RUBY_TYPED_FREE_IMMEDIATELY \
	};

#define DEFINE_GOBJECT_CHILD_KLASS_DATA_TYPE(klass) \
	static const rb_data_type_t klass##_dt = { \
		.wrap_struct_name = "Frida::" #klass "Data", \
		.function = { \
			.dcompact = NULL, \
			.dmark = NULL, \
			.dsize = NULL, \
			.dfree = (void (*)(void*))klass##_free, \
		}, \
		.parent = &GObject_dt, \
		.flags = RUBY_TYPED_FREE_IMMEDIATELY \
	};

#define MAKE_DATA(k) \
	k##_d *d; \
	VALUE obj = TypedData_Make_Struct(klass, k##_d, &k##_dt, d);

#define MAKE_DATA_EX(k, var) \
	k##_d *var; \
	VALUE obj = TypedData_Make_Struct(klass, k##_d, &k##_dt, var);

#define GET_DATA(klass) \
	klass##_d *d; \
    TypedData_Get_Struct(self, klass##_d, &klass##_dt, d);

#define GET_DATA_EX(klass, self, var) \
	klass##_d *var; \
    TypedData_Get_Struct(self, klass##_d, &klass##_dt, var);

#define GET_GOBJECT_DATA() \
    GET_DATA(GObject);

#define REQUIRE_GOBJECT_HANDLE() \
    if (!d->handle) return (Qnil);

#define CALL_GVL_FREE(func, arg) \
    rb_thread_call_without_gvl((void_fp)func##_gvl_free, arg, NULL, NULL);

typedef struct {
    void	*ret;
    GError	*gerr;
} gvl_free_proxy_result;

#define GVL_FREE_PROXY_FUNC(name, arg) \
    static gvl_free_proxy_result *name##_gvl_free(arg)

#define GERROR_BLOCK \
    gerror: \
        raise_rerror(NULL, _gerr); \
        return (Qnil);

#define UNREACHABLE_GERROR_BLOCK GERROR_BLOCK

#define RETURN_GVL_FREE_RESULT(_ret) \
	gvl_free_proxy_result *result = malloc(sizeof(gvl_free_proxy_result)); \
	result->ret = _ret; \
	result->gerr = gerr; \
	return (result);

#define CALL_GVL_FREE_WITH_RET(_ret, func, arg) \
    gvl_free_proxy_result *result = (gvl_free_proxy_result *)rb_thread_call_without_gvl((void_fp)func##_gvl_free, arg, NULL, NULL); \
    _ret = result->ret; \
    GError *_gerr = result->gerr; \
    free(result); \
    if (_gerr) goto gerror;

#define TO_RBCLOSURE(c) ((RBClosure *)c)

typedef void *(*void_fp)(void *);

void	Init_c_frida(void);
void	raise_rerror(char *err, GError *gerr);
void	raise_argerror(char *err);

#endif
