#include "Crash.h"

VALUE	Crash_from_FridaCrash(FridaCrash *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cCrash);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = g_object_unref;
    rb_ivar_set(self, rb_intern("pid"), LL2NUM(frida_crash_get_pid(handle)));
    rb_ivar_set(self, rb_intern("process_name"), rbGObject_marshal_string(frida_crash_get_process_name(handle)));
    rb_ivar_set(self, rb_intern("summary"), rbGObject_marshal_string(frida_crash_get_summary(handle)));
    rb_ivar_set(self, rb_intern("report"), rbGObject_marshal_string(frida_crash_get_report(handle)));
    rb_ivar_set(self, rb_intern("parameters"), rbGObject_marshal_dict(frida_crash_get_parameters(handle)));
    return (self);
}

VALUE Crash_inspect(VALUE self)
{
    VALUE s, report;

    report = rb_funcall(self, rb_intern("report"), 0, NULL);
    s = rb_sprintf("#<Crash: pid=%+"PRIsVALUE", process_name=%+"PRIsVALUE", summary=%+"PRIsVALUE", report=<%+"PRIsVALUE" bytes>, parameters=%+"PRIsVALUE">", \
                   rb_funcall(self, rb_intern("pid"), 0, NULL),
                   rb_funcall(self, rb_intern("process_name"), 0, NULL),
                   rb_funcall(self, rb_intern("summary"), 0, NULL),
                   (report == Qnil) ? UINT2NUM(0) : rb_funcall(report, rb_intern("length"), 0, NULL),
                   rb_funcall(self, rb_intern("parameters"), 0, NULL)
                  );
    return (s);
}

/*
    call-seq:
        #pid() -> Fixnum
*/
static VALUE Crash_pid(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("pid")));
}

/*
    call-seq:
        #process_name() -> String
*/
static VALUE Crash_process_name(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("process_name")));
}

/*
    call-seq:
        #summary() -> String
*/
static VALUE Crash_summary(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("summary")));
}

/*
    call-seq:
        #report() -> String
*/
static VALUE Crash_report(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("report")));
}

/*
    call-seq:
        #parameters() -> Hash
*/
static VALUE Crash_parameters(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("parameters")));
}

void	define_Crash()
{
    cCrash = rb_define_class_under(mCFrida, "Crash", cGObject);

    rb_define_method(cCrash, "inspect", Crash_inspect, 0);
    rb_define_alias(cCrash, "to_s", "inspect");
    rb_define_method(cCrash, "pid", Crash_pid, 0);
    rb_define_method(cCrash, "process_name", Crash_process_name, 0);
    rb_define_method(cCrash, "summary", Crash_summary, 0);
    rb_define_method(cCrash, "report", Crash_report, 0);
    rb_define_method(cCrash, "parameters", Crash_parameters, 0);
}