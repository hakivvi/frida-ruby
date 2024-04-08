#include "Child.h"

VALUE	Child_from_FridaChild(FridaChild *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    self = rb_class_new_instance(0, NULL, cChild);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = g_object_unref;
    rb_ivar_set(self, rb_intern("pid"), UINT2NUM(frida_child_get_pid(d->handle)));
    rb_ivar_set(self, rb_intern("parent_pid"), UINT2NUM(frida_child_get_parent_pid(d->handle)));
    rb_ivar_set(self, rb_intern("identifier"), rbGObject_marshal_string(frida_child_get_identifier(d->handle)));
    rb_ivar_set(self, rb_intern("path"), rbGObject_marshal_string(frida_child_get_path(d->handle)));
    rb_ivar_set(self, rb_intern("origin"), rbGObject_marshal_enum(frida_child_get_origin(d->handle), FRIDA_TYPE_CHILD_ORIGIN));
    gint len;
    rb_ivar_set(self, rb_intern("argv"), rbGObject_marshal_strv((gchar **)frida_child_get_argv(d->handle, &len), len));
    rb_ivar_set(self, rb_intern("envp"), rbGObject_marshal_envp((const gchar **)frida_child_get_envp(d->handle, &len), len));
    return (self);
}

static VALUE Child_inspect(VALUE self)
{
    GET_GOBJECT_DATA();

    VALUE s;
    FridaChildOrigin origin;
    GString *inspect_s;

    inspect_s = g_string_new("#<Child: ");
    g_string_append(inspect_s, "pid=%+"PRIsVALUE", parent_pid=%+"PRIsVALUE);
    g_string_append(inspect_s, ", origin=%+"PRIsVALUE", identifier=%+"PRIsVALUE);

    if (d->handle)
        origin = frida_child_get_origin(d->handle);
    else
        origin = FRIDA_CHILD_ORIGIN_FORK;
    if (origin != FRIDA_CHILD_ORIGIN_FORK) {
        g_string_append(inspect_s, ", path=%+"PRIsVALUE", argv=%+"PRIsVALUE", envp=%+"PRIsVALUE);
        g_string_append(inspect_s, ">");
        s = rb_sprintf(inspect_s->str, \
                       rb_funcall(self, rb_intern("pid"), 0, NULL),  rb_funcall(self, rb_intern("parent_pid"), 0, NULL), \
                       rb_funcall(self, rb_intern("origin"), 0, NULL),  rb_funcall(self, rb_intern("identifier"), 0, NULL), \
                       rb_funcall(self, rb_intern("path"), 0, NULL),  rb_funcall(self, rb_intern("argv"), 0, NULL), \
                       rb_funcall(self, rb_intern("envp"), 0, NULL));
    } else {
        g_string_append(inspect_s, ">");
        s = rb_sprintf(inspect_s->str, \
                       rb_funcall(self, rb_intern("pid"), 0, NULL),  rb_funcall(self, rb_intern("parent_pid"), 0, NULL), \
                       rb_funcall(self, rb_intern("origin"), 0, NULL),  rb_funcall(self, rb_intern("identifier"), 0, NULL));
    }
    g_string_free(inspect_s, TRUE);
    return (s);
}

/*
    call-seq:
        #pid() -> Fixnum
*/
static VALUE Child_pid(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("pid")));
}

/*
    call-seq:
        #identifier() -> String
*/
static VALUE Child_identifier(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("identifier")));
}

/*
    call-seq:
        #parent_pid() -> Fixnum
*/
static VALUE Child_parent_pid(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("parent_pid")));
}

/*
    call-seq:
        #path() -> String
*/
static VALUE Child_path(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("path")));
}

/*
    call-seq:
        #argv() -> Array
*/
static VALUE Child_argv(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("argv")));
}

/*
    call-seq:
        #envp() -> Array
*/
static VALUE Child_envp(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("envp")));
}

/*
    call-seq:
        #origin() -> String
*/
static VALUE Child_origin(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("origin")));
}

void	define_Child()
{
    cChild = rb_define_class_under(mCFrida, "Child", cGObject);

    rb_define_method(cChild, "inspect", Child_inspect, 0);
    rb_define_alias(cChild, "to_s", "inspect");
    rb_define_method(cChild, "pid", Child_pid, 0);
    rb_define_method(cChild, "parent_pid", Child_parent_pid, 0);
    rb_define_method(cChild, "identifier", Child_identifier, 0);
    rb_define_method(cChild, "origin", Child_origin, 0);
    rb_define_method(cChild, "path", Child_path, 0);
    rb_define_method(cChild, "argv", Child_argv, 0);
    rb_define_method(cChild, "envp", Child_envp, 0);
}
