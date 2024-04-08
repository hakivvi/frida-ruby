#include "Relay.h"

VALUE Relay_from_FridaRelay(FridaRelay *handle)
{
    VALUE self;

    if (!handle)
        return (Qnil);
    VALUE args[4] = {Qfalse};
    self = rb_class_new_instance(4, args, cRelay);
    GET_GOBJECT_DATA();
    d->handle = handle;
    d->destroy = g_object_unref;
    rb_ivar_set(self, rb_intern("address"), rbGObject_marshal_string(frida_relay_get_address(handle)));
    rb_ivar_set(self, rb_intern("username"), rbGObject_marshal_string(frida_relay_get_username(handle)));
    rb_ivar_set(self, rb_intern("password"), rbGObject_marshal_string(frida_relay_get_password(handle)));
    rb_ivar_set(self, rb_intern("kind"), rbGObject_marshal_enum(frida_relay_get_kind(handle), FRIDA_TYPE_RELAY_KIND));
    return (self);
}

static VALUE Relay_inspect(VALUE self)
{
    VALUE s;

    s = rb_sprintf("#<Relay: address=%+"PRIsVALUE", username=%+"PRIsVALUE", password=%+"PRIsVALUE", kind=%+"PRIsVALUE">", \
                   rb_funcall(self, rb_intern("address"), 0, NULL),  rb_funcall(self, rb_intern("username"), 0, NULL), \
                   rb_funcall(self, rb_intern("password"), 0, NULL),  rb_funcall(self, rb_intern("kind"), 0, NULL));
    return (s);
}

/*
    call-seq:
        #address() -> String
*/
static VALUE Relay_address(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("address")));
}

/*
    call-seq:
        #username() -> String
*/
static VALUE Relay_username(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("username")));
}

/*
    call-seq:
        #password() -> String
*/
static VALUE Relay_password(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("password")));
}

/*
    call-seq:
        #kind() -> String
*/
static VALUE Relay_kind(VALUE self)
{
    return (rb_ivar_get(self, rb_intern("kind")));
}

/*
    call-seq:
        #new(address, username, password, kind) -> Relay
*/
static VALUE Relay_initialize(VALUE self, VALUE address, VALUE username, VALUE password, VALUE kind)
{
    GET_GOBJECT_DATA();
    FridaRelayKind	gkind;
    FridaRelay		*handle;

    if (address == Qfalse)
        return (self);
    if (!RB_TYPE_P(address, T_STRING)) {
        raise_argerror("address should be a string.");
        return (Qnil);
    }
    if (!RB_TYPE_P(username, T_STRING)) {
        raise_argerror("username should be a string.");
        return (Qnil);
    }
    if (!RB_TYPE_P(password, T_STRING)) {
        raise_argerror("password should be a string.");
        return (Qnil);
    }
    if (!RB_TYPE_P(kind, T_STRING)) {
        raise_argerror("kind should be a string.");
        return (Qnil);
    }
    if (!rbGObject_unmarshal_enum(StringValueCStr(kind), FRIDA_TYPE_RELAY_KIND, &gkind)) {
        raise_argerror("kind is not valid.");
        return (Qnil);
    }
    handle = frida_relay_new(StringValueCStr(address), StringValueCStr(username), StringValueCStr(password), gkind);
    d->handle = handle;
    d->destroy = g_object_unref;
    rb_ivar_set(self, rb_intern("address"), rbGObject_marshal_string(frida_relay_get_address(handle)));
    rb_ivar_set(self, rb_intern("username"), rbGObject_marshal_string(frida_relay_get_username(handle)));
    rb_ivar_set(self, rb_intern("password"), rbGObject_marshal_string(frida_relay_get_password(handle)));
    rb_ivar_set(self, rb_intern("kind"), rbGObject_marshal_enum(frida_relay_get_kind(handle), FRIDA_TYPE_RELAY_KIND));
    return (self);
}

void	define_Relay()
{
    cRelay = rb_define_class_under(mCFrida, "Relay", cGObject);

    rb_define_method(cRelay, "inspect", Relay_inspect, 0);
    rb_define_alias(cRelay, "to_s", "inspect");

    rb_define_method(cRelay, "initialize", Relay_initialize, 4);
    rb_define_method(cRelay, "address", Relay_address, 0);
    rb_define_method(cRelay, "username", Relay_username, 0);
    rb_define_method(cRelay, "password", Relay_password, 0);
    rb_define_method(cRelay, "kind", Relay_kind, 0);
}
