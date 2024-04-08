#include "Spawn.h"

VALUE	Spawn_from_FridaSpawn(FridaSpawn *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cSpawn);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = g_object_unref;
    rb_ivar_set(self, rb_intern("pid"), UINT2NUM(frida_spawn_get_pid(d->handle)));
    rb_ivar_set(self, rb_intern("identifier"), rbGObject_marshal_string(frida_spawn_get_identifier(d->handle)));
    return (self);
}

static VALUE Spawn_inspect(VALUE self)
{
    VALUE s;

    s = rb_sprintf("#<Spawn: pid=%+"PRIsVALUE", identifier=%+"PRIsVALUE">", \
                   rb_funcall(self, rb_intern("pid"), 0, NULL),  rb_funcall(self, rb_intern("identifier"), 0, NULL));
    return (s);
}

/*
    call-seq:
        #pid() -> Fixnum
*/
static VALUE Spawn_pid(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("pid")));
}

/*
    call-seq:
        #identifier() -> String
*/
static VALUE Spawn_identifier(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("identifier")));
}

void	define_Spawn()
{
    cSpawn = rb_define_class_under(mCFrida, "Spawn", cGObject);

    rb_define_method(cSpawn, "inspect", Spawn_inspect, 0);
    rb_define_alias(cSpawn, "to_s", "inspect");
    rb_define_method(cSpawn, "pid", Spawn_pid, 0);
    rb_define_method(cSpawn, "identifier", Spawn_identifier, 0);
}
