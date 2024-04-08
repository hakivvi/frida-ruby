#include "Application.h"

VALUE	Application_from_FridaApplication(FridaApplication *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cApplication);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = g_object_unref;
    rb_ivar_set(self, rb_intern("identifier"), rb_str_new_cstr(frida_application_get_identifier(d->handle)));
    rb_ivar_set(self, rb_intern("name"), rb_str_new_cstr(frida_application_get_name(d->handle)));
    rb_ivar_set(self, rb_intern("pid"), LL2NUM(frida_application_get_pid(d->handle)));
    rb_ivar_set(self, rb_intern("parameters"), rbApplication_marshal_parameters_dict(frida_application_get_parameters(d->handle)));
    return (self);
}

static VALUE Application_inspect(VALUE self)
{
    VALUE s;

    s = rb_sprintf("#<Application: identifier=%+"PRIsVALUE", name=%+"PRIsVALUE", pid=%+"PRIsVALUE", parameters=%+"PRIsVALUE">", \
                   rb_funcall(self, rb_intern("identifier"), 0, NULL),
                   rb_funcall(self, rb_intern("name"), 0, NULL),
                   rb_funcall(self, rb_intern("pid"), 0, NULL),
                   rb_funcall(self, rb_intern("parameters"), 0, NULL)
                  );
    return (s);
}

/*
    call-seq:
        #identifier() -> String
*/
static VALUE Application_identifier(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("identifier")));
}

/*
    call-seq:
        #name() -> String
*/
static VALUE Application_name(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("name")));
}

/*
    call-seq:
        #pid() -> Fixnum
*/
static VALUE Application_pid(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("pid")));
}

/*
    call-seq:
        #parameters() -> Hash
*/
static VALUE Application_parameters(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("parameters")));
}

void	define_Application()
{
    cApplication = rb_define_class_under(mCFrida, "Application", cGObject);

    rb_define_method(cApplication, "inspect", Application_inspect, 0);
    rb_define_alias(cApplication, "to_s", "inspect");
    rb_define_method(cApplication, "identifier", Application_identifier, 0);
    rb_define_method(cApplication, "name", Application_name, 0);
    rb_define_method(cApplication, "pid", Application_pid, 0);
    rb_define_method(cApplication, "parameters", Application_parameters, 0);
}
