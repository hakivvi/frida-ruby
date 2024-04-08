#include "GObject.h"

DEFINE_KLASS_DATA_TYPE(GObject);

void	GObject_free(GObject_d *d)
{
    if (d->handle)
        d->destroy(d->handle);
    xfree(d);
}

/*
    call-seq:
        #on(signal_name) {} -> nil
        #on(signal_name, callback) -> nil
*/
static VALUE GObject_on(int argc, VALUE *argv, VALUE self)
{
    GET_DATA(GObject);
    REQUIRE_GOBJECT_HANDLE();
    VALUE signal_name, callback;
    GType self_type = G_OBJECT_TYPE(d->handle);
    guint signal_id;
    bool is_lambda;
    GSignalQuery squery;

    if (rb_block_given_p()) {
        // blocks are handy however we cannot compare them because they are unique
        // so you cannot remove them later!
        rb_scan_args(argc, argv, "1&", &signal_name, &callback);
    } else {
        rb_scan_args(argc, argv, "2", &signal_name, &callback);
        // either a Method or Proc object.
        if (!rb_respond_to(callback, rb_intern("call"))) {
            raise_argerror("callback should be callable.");
            return (Qnil);
        }
    }
    if (!RB_TYPE_P(signal_name, T_STRING)) {
        raise_argerror("signal name must be a string.");
        return (Qnil);
    }
    signal_id = g_signal_lookup(StringValueCStr(signal_name), self_type);
    if (!signal_id) {
        raise_argerror("invalid signal name.");
        return (Qnil);
    }
    is_lambda = (rb_obj_is_kind_of(callback, rb_const_get(rb_cObject, rb_intern("Method"))) == Qtrue) ? true : (rb_funcall(callback, rb_intern("lambda?"), 0, NULL) == Qtrue);
    int arity = NUM2INT(rb_funcall(callback, rb_intern("arity"), 0, NULL));
    g_signal_query(signal_id, &squery);
    if (is_lambda && arity > (int)(squery.n_params + 1)) {
        gchar *err = g_strdup_printf("callback expects too many arguments, signal expects %d.", squery.n_params);
        raise_argerror(err);
        g_free(err);
        return (Qnil);
    }
    GClosure *closure = g_closure_new_simple(sizeof(RBClosure), (void*)callback);
    g_closure_set_marshal(closure, (GClosureMarshal)gvl_bridge_forward_GC);
    TO_RBCLOSURE(closure)->is_lambda = is_lambda;
    TO_RBCLOSURE(closure)->signal_id = signal_id;
    TO_RBCLOSURE(closure)->arity = arity;
    g_signal_connect_closure_by_id(d->handle, signal_id, 0, closure, TRUE);
    // objects generated on the fly (thus not referenced), e.g blocks `#on("signal", :id) {block}` / `#on("signal", &callable)`
    // or Methods `#on("signal", method(:defined_fn))` or Procs `#on("signal", ->{})`
    // will be picked up by the GC, since in Ruby's view there is no more usage/reference to them
    // so we keep them in this hidden Hash.
    if (rb_hash_aref(rb_iv_get(self, "callbacks"), signal_name) == Qnil)
        rb_hash_aset(rb_iv_get(self, "callbacks"), signal_name, rb_ary_new());
    rb_ary_push(rb_hash_aref(rb_iv_get(self, "callbacks"), signal_name), callback);
    d->signal_closures = g_slist_prepend(d->signal_closures, closure);
    return (Qnil);
}

static gint compare_callback(RBClosure *closure, VALUE callback)
{
    return (rb_equal((VALUE)closure->gclosure.data, callback) == Qtrue ? 0 : -1);
}

/*
    call-seq:
        #off(signal_name, callback) -> nil
*/
static VALUE GObject_off(int argc, VALUE *argv, VALUE self)
{
    GET_DATA(GObject);
    REQUIRE_GOBJECT_HANDLE();
    VALUE signal_name, callback;
    GType self_type = G_OBJECT_TYPE(d->handle);
    guint signal_id;
    GSList *callback_entry;
    int	is_block = rb_block_given_p();

    // remember you cannot remove Proc objects that you do not hold reference to!
    rb_scan_args(argc, argv, "2", &signal_name, &callback);
    if (!RB_TYPE_P(signal_name, T_STRING)) {
        raise_argerror("signal name must be a string.");
        return (Qnil);
    }
    signal_id = g_signal_lookup(StringValueCStr(signal_name), self_type);
    if (!signal_id) {
        raise_argerror("invalid signal name.");
        return (Qnil);
    }
    if (rb_respond_to(callback, rb_intern("call"))) {
        callback_entry = g_slist_find_custom(d->signal_closures, (gpointer)callback, (GCompareFunc)compare_callback);
        if (!callback_entry) {
            raise_argerror("callback not found.");
            return (Qnil);
        }
    } else {
        raise_argerror("second argument should be callable.");
        return (Qnil);
    }
    GClosure *closure = callback_entry->data;
    d->signal_closures = g_slist_delete_link(d->signal_closures, callback_entry);
    g_signal_handlers_disconnect_matched(d->handle, G_SIGNAL_MATCH_CLOSURE, signal_id, 0, closure, NULL, NULL);
    // this will delete a duplicate callback for a signal, but that is fine ?
    rb_ary_delete(rb_hash_aref(rb_iv_get(self, "callbacks"), signal_name), callback);
    return (Qnil);
}

static VALUE GObject_alloc(VALUE klass)
{
    MAKE_DATA(GObject);
    d->handle = NULL;
    d->destroy = NULL;
    d->signal_closures = NULL;
    rb_ivar_set(obj, rb_intern("callbacks"), rb_hash_new());
    return (obj);
}

void	define_GObject()
{
    cGObject = rb_define_class_under(mCFrida, "GObject", rb_cObject);
    rb_define_alloc_func(cGObject, GObject_alloc);
    rb_define_method(cGObject, "on", GObject_on, -1);
    rb_define_method(cGObject, "off", GObject_off, -1);
}
