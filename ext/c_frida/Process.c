#include "Process.h"

VALUE	Process_from_FridaProcess(FridaProcess *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cProcess);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = g_object_unref;
    rb_ivar_set(self, rb_intern("pid"), LL2NUM(frida_process_get_pid(d->handle)));
    rb_ivar_set(self, rb_intern("name"), rb_str_new_cstr(frida_process_get_name(d->handle)));
    rb_ivar_set(self, rb_intern("parameters"), rbProcess_marshal_parameters_dict(frida_process_get_parameters(d->handle)));
    return (self);
}

static VALUE Process_inspect(VALUE self)
{
    VALUE s;

    s = rb_sprintf("#<Process: name=%+"PRIsVALUE", pid=%+"PRIsVALUE", parameters=%+"PRIsVALUE">", \
                   rb_funcall(self, rb_intern("name"), 0, NULL),
                   rb_funcall(self, rb_intern("pid"), 0, NULL),
                   rb_funcall(self, rb_intern("parameters"), 0, NULL)
                  );
    return (s);
}

/*
    call-seq:
        #pid() -> Fixnum
*/
static VALUE Process_pid(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("pid")));
}

/*
    call-seq:
        #name() -> String
*/
static VALUE Process_name(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("name")));
}

/*
    call-seq:
        #parameters() -> Hash
*/
static VALUE Process_parameters(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("parameters")));
}

void	define_Process()
{
    cProcess = rb_define_class_under(mCFrida, "Process", cGObject);

    rb_define_method(cProcess, "inspect", Process_inspect, 0);
    rb_define_alias(cProcess, "to_s", "inspect");
    rb_define_method(cProcess, "pid", Process_pid, 0);
    rb_define_method(cProcess, "name", Process_name, 0);
    rb_define_method(cProcess, "parameters", Process_parameters, 0);
}
