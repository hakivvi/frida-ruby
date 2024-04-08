#include "IOStream.h"

DEFINE_GOBJECT_CHILD_KLASS_DATA_TYPE(IOStream);

VALUE	IOStream_from_GIOStream(GIOStream *stream)
{
    VALUE self;

    if (!stream) return (Qnil);
    self = rb_class_new_instance(0, NULL, cIOStream);
    GET_GOBJECT_DATA();
    d->handle = stream;
    GET_DATA_EX(IOStream, self, io_d);
    io_d->input = g_io_stream_get_input_stream(d->handle);
    io_d->output = g_io_stream_get_output_stream(d->handle);
    return (self);
}

void IOStream_free(IOStream_d *d)
{
    if (d->base.handle)
        g_object_unref(d->base.handle);
    xfree(d);
}

/*
    call-seq:
        #is_closed() -> [TrueClass, FalseClass]
*/
static VALUE IOStream_is_closed(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    return (g_io_stream_is_closed(d->handle) ? Qtrue : Qfalse);
}

static VALUE IOStream_inspect(VALUE self)
{
    GET_GOBJECT_DATA();
    VALUE s;

    s = rb_sprintf("#<IOStream: handle=\"%p\", is_closed=%+"PRIsVALUE">", d->handle, rb_funcall(self, rb_intern("is_closed"), 0, NULL));
    return (s);
}

GVL_FREE_PROXY_FUNC(write, write_proxy_args *args)
{
    GError *gerr = NULL;
    gsize written;

    written = g_output_stream_write(args->output, args->data, args->size, NULL, &gerr);
    RETURN_GVL_FREE_RESULT((void*)written);
}

/*
    call-seq:
        #write(data) -> Fixnum
*/
static VALUE IOStream_write(VALUE self, VALUE data)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    GET_DATA_EX(IOStream, self, io_d);

    if (!RB_TYPE_P(data, T_STRING)) {
        raise_argerror("data should be a string.");
        return (Qnil);
    }
    write_proxy_args args = {
        .output = io_d->output,
        .data = RSTRING_PTR(data),
        .size = RSTRING_LEN(data)
    };
    CALL_GVL_FREE_WITH_RET(void *written, write, &args);
    return (ULONG2NUM((gsize)written));

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(write_all, write_proxy_args *args)
{
    GError *gerr = NULL;

    g_output_stream_write_all(args->output, args->data, args->size, NULL, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #write_all(data) -> nil
*/
static VALUE IOStream_write_all(VALUE self, VALUE data)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    GET_DATA_EX(IOStream, self, io_d);

    if (!RB_TYPE_P(data, T_STRING)) {
        raise_argerror("data should be a string.");
        return (Qnil);
    }
    write_proxy_args args = {
        .output = io_d->output,
        .data = RSTRING_PTR(data),
        .size = RSTRING_LEN(data)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, write_all, &args);
    return (Qnil);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(read, read_proxy_args *args)
{
    GError *gerr = NULL;
    gsize read;

    read = g_input_stream_read(args->input, args->buffer, args->count, NULL, &gerr);
    RETURN_GVL_FREE_RESULT((void*)read);
}

/*
    call-seq:
        #read(count) -> String
*/
static VALUE IOStream_read(VALUE self, VALUE count)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    GET_DATA_EX(IOStream, self, io_d);

    if (!FIXNUM_P(count)) {
        raise_argerror("count should be a number.");
        return (Qnil);
    }
    VALUE buffer = rb_str_new(NULL, NUM2ULONG(count));
    read_proxy_args args = {
        .input = io_d->input,
        .buffer = RSTRING_PTR(buffer),
        .count = NUM2ULONG(count)
    };
    CALL_GVL_FREE_WITH_RET(void *read, read, &args);
    if ((gsize)read != NUM2ULONG(count))
        buffer = rb_str_new(RSTRING_PTR(buffer), (gsize)read);
    return (buffer);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(read_all, read_proxy_args *args)
{
    GError *gerr = NULL;
    gsize	read;

    g_input_stream_read_all(args->input, args->buffer, args->count, &read, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #read_all(count) -> String
*/
static VALUE IOStream_read_all(VALUE self, VALUE count)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();
    GET_DATA_EX(IOStream, self, io_d);

    if (!FIXNUM_P(count)) {
        raise_argerror("count should be a number.");
        return (Qnil);
    }
    VALUE buffer = rb_str_new(NULL, NUM2ULONG(count));
    read_proxy_args args = {
        .input = io_d->input,
        .buffer = RSTRING_PTR(buffer),
        .count = NUM2ULONG(count)
    };
    CALL_GVL_FREE_WITH_RET(void *dummy, read_all, &args);
    return (buffer);

    GERROR_BLOCK
}

GVL_FREE_PROXY_FUNC(close, GIOStream *handle)
{
    GError *gerr = NULL;

    g_io_stream_close(handle, NULL, &gerr);
    RETURN_GVL_FREE_RESULT(NULL);
}

/*
    call-seq:
        #close() -> nil
*/
static VALUE IOStream_close(VALUE self)
{
    GET_GOBJECT_DATA();
    REQUIRE_GOBJECT_HANDLE();

    CALL_GVL_FREE_WITH_RET(void *dummy, close, d->handle);
    return (Qnil);

    GERROR_BLOCK
}

static VALUE IOStream_alloc(VALUE klass)
{
    MAKE_DATA(IOStream);
    memset(d, 0, sizeof(IOStream_d));
    rb_ivar_set(obj, rb_intern("callbacks"), rb_hash_new());
    return (obj);
}

void	define_IOStream()
{
    cIOStream = rb_define_class_under(mCFrida, "IOStream", cGObject);
    rb_define_alloc_func(cIOStream, IOStream_alloc);
    rb_define_method(cIOStream, "inspect", IOStream_inspect, 0);
    rb_define_alias(cIOStream, "to_s", "inspect");
    rb_define_method(cIOStream, "is_closed", IOStream_is_closed, 0);
    rb_define_method(cIOStream, "read", IOStream_read, 1);
    rb_define_method(cIOStream, "read_all", IOStream_read_all, 1);
    rb_define_method(cIOStream, "write", IOStream_write, 1);
    rb_define_method(cIOStream, "write_all", IOStream_write_all, 1);
    rb_define_method(cIOStream, "close", IOStream_close, 0);
}
