#include "EndpointParameters.h"

VALUE	EndpointParameters_from_FridaEndpointParameters(FridaEndpointParameters *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cEndpointParameters);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = g_object_unref;
    return (self);
}

static void frida_rb_authentication_service_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_EXTENDED (FridaRBAuthenticationService, frida_rb_authentication_service, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (FRIDA_TYPE_AUTHENTICATION_SERVICE, frida_rb_authentication_service_iface_init))

static void frida_rb_authentication_service_dispose (GObject * object);
static void frida_rb_authentication_service_authenticate (FridaAuthenticationService * service, const gchar * token, GCancellable * cancellable,
        GAsyncReadyCallback callback, gpointer user_data);
static gchar *frida_rb_authentication_service_authenticate_finish (FridaAuthenticationService * service, GAsyncResult * result, GError ** error);

static FridaRBAuthenticationService *frida_rb_authentication_service_new(VALUE callback)
{
    FridaRBAuthenticationService *service;

    service = g_object_new(FRIDA_TYPE_RB_AUTHENTICATION_SERVICE, NULL);
    service->callback = callback;

    return (service);
}

static void frida_rb_authentication_service_class_init(FridaRBAuthenticationServiceClass * klass)
{
    GObjectClass * object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = frida_rb_authentication_service_dispose;
}

static void frida_rb_authentication_service_iface_init (gpointer g_iface, gpointer iface_data)
{
    puts("frida_rb_authentication_service_iface_init");
    FridaAuthenticationServiceIface * iface = g_iface;

    iface->authenticate = frida_rb_authentication_service_authenticate;
    iface->authenticate_finish = frida_rb_authentication_service_authenticate_finish;
}

static void frida_rb_authentication_service_init (FridaRBAuthenticationService * self)
{
    self->pool = g_thread_pool_new ((GFunc) gvl_bridge_forward_GT, self, 1, FALSE, NULL);
}

static void frida_rb_authentication_service_dispose (GObject * object)
{
    FridaRBAuthenticationService * self = FRIDA_RB_AUTHENTICATION_SERVICE (object);

    if (self->pool != NULL) {
        g_thread_pool_free (self->pool, FALSE, FALSE);
        self->pool = NULL;
    }

    G_OBJECT_CLASS (frida_rb_authentication_service_parent_class)->dispose (object);
}

static void frida_rb_authentication_service_authenticate (FridaAuthenticationService * service, const gchar * token, GCancellable * cancellable,
        GAsyncReadyCallback callback, gpointer user_data)
{
    FridaRBAuthenticationService * self;
    GTask * task;

    self = FRIDA_RB_AUTHENTICATION_SERVICE (service);

    task = g_task_new (self, cancellable, callback, user_data);
    g_task_set_task_data (task, g_strdup (token), g_free);

    g_thread_pool_push (self->pool, task, NULL);
}

static gchar *frida_rb_authentication_service_authenticate_finish (FridaAuthenticationService * service, GAsyncResult * result, GError ** error)
{
    return g_task_propagate_pointer (G_TASK (result), error);
}

/*
    call-seq:
        #new(address:, port:, certificate:, origin:, auth_token:, auth_callback:, asset_root:) -> EndpointParameters
*/
static VALUE EndpointParameters_initialize(int argc, VALUE *argv, VALUE self)
{
    GET_GOBJECT_DATA();
    GTlsCertificate *certificate = NULL;
    FridaAuthenticationService *auth_service = NULL;
    GFile *asset_root = NULL;
    unsigned short port = 0;
    char *address = NULL, *origin = NULL, *auth_token = NULL;
    VALUE kws;

    rb_scan_args(argc, argv, ":", &kws);
    if (!NIL_P(kws)) {
        VALUE raddress = rb_hash_aref(kws, ID2SYM(rb_intern("address")));
        if (!NIL_P(raddress)) {
            if (!RB_TYPE_P(raddress, T_STRING)) {
                raise_argerror("address must be a string.");
                goto invalid_arg;
            }
            address = StringValueCStr(raddress);
        }
        VALUE rport = rb_hash_aref(kws, ID2SYM(rb_intern("port")));
        if (!NIL_P(rport)) {
            if (!RB_TYPE_P(rport, T_FIXNUM)) {
                raise_argerror("port must be a number.");
                goto invalid_arg;
            }
            port = (unsigned short)NUM2USHORT(rport);
        }
        VALUE rcert = rb_hash_aref(kws, ID2SYM(rb_intern("certificate")));
        if (!NIL_P(rcert)) {
            if (!RB_TYPE_P(rcert, T_STRING)) {
                raise_argerror("certificate must be a string.");
                goto invalid_arg;
            }
            char *cert = StringValueCStr(rcert);
            if (!rbGObject_unmarshal_certificate(cert, &certificate)) {
                raise_argerror("invalid certificate.");
                goto invalid_arg;
            }
        }
        VALUE rorigin = rb_hash_aref(kws, ID2SYM(rb_intern("origin")));
        if (!NIL_P(rorigin)) {
            if (!RB_TYPE_P(rorigin, T_STRING)) {
                raise_argerror("origin must be a string.");
                goto invalid_arg;
            }
            origin = StringValueCStr(rorigin);
        }
        VALUE rauth_token = rb_hash_aref(kws, ID2SYM(rb_intern("auth_token")));
        if (!NIL_P(rauth_token)) {
            if (!RB_TYPE_P(rauth_token, T_STRING)) {
                raise_argerror("auth_token must be a string.");
                goto invalid_arg;
            }
            auth_token = StringValueCStr(rauth_token);
        }
        VALUE rauth_callback = rb_hash_aref(kws, ID2SYM(rb_intern("auth_callback")));
        if (!NIL_P(rauth_callback)) {
            // currently support only methods.
            if (rb_obj_is_kind_of(rauth_callback, rb_const_get(rb_cObject, rb_intern("Method"))) == Qfalse || \
                NUM2INT(rb_funcall(rauth_callback, rb_intern("arity"), 0, NULL)) != 1) {
                raise_argerror("auth_callback must be a method with one argument.");
                goto invalid_arg;
            }
            // don't GC
            rb_ivar_set(self, rb_intern("auth_callback"), rauth_callback);
        }
        if (auth_token)
            auth_service = FRIDA_AUTHENTICATION_SERVICE(frida_static_authentication_service_new(auth_token));
        else if (rauth_callback != Qnil)
            auth_service = FRIDA_AUTHENTICATION_SERVICE(frida_rb_authentication_service_new(rauth_callback));
        VALUE rasset_root = rb_hash_aref(kws, ID2SYM(rb_intern("asset_root")));
        if (!NIL_P(rasset_root)) {
            if (!RB_TYPE_P(rasset_root, T_STRING)) {
                raise_argerror("asset_root must be a string.");
                goto invalid_arg;
            }
            asset_root = g_file_new_for_path(StringValueCStr(rasset_root));
        }
    } else
        return (self);
    d->handle = frida_endpoint_parameters_new(address, port, certificate, origin, auth_service, asset_root);
    d->destroy = g_object_unref;
    return (self);

invalid_arg:
    g_clear_object(&asset_root);
    g_clear_object(&auth_service);
    g_clear_object(&certificate);
    return (Qnil);
}

void	define_EndpointParameters()
{
    cEndpointParameters = rb_define_class_under(mCFrida, "EndpointParameters", cGObject);

    rb_define_method(cEndpointParameters, "initialize", EndpointParameters_initialize, -1);
}
