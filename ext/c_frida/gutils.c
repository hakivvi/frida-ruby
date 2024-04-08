#include "gutils.h"

// Ruby version of frida-python's Glib objects un/marshals.
VALUE rbGObject_marshal_string(const gchar * str)
{
    if (!str)
        return (Qnil);
    return (rb_str_new_cstr(str));
}

gboolean rbGObject_unmarshal_string(VALUE value, gchar **str)
{
    VALUE to_s = rb_funcall(value, rb_intern("to_s"), 0);
    *str = g_strdup(StringValueCStr(to_s));
    return (*str != NULL);
}

VALUE rbGObject_marshal_enum(gint value, GType type)
{
    GEnumClass * enum_class;
    GEnumValue * enum_value;
    VALUE	result;

    enum_class = g_type_class_ref (type);

    enum_value = g_enum_get_value (enum_class, value);
    g_assert (enum_value != NULL);

    result = rbGObject_marshal_string(enum_value->value_nick);

    g_type_class_unref (enum_class);

    return (result);
}

VALUE rbGObject_marshal_variant(GVariant * variant)
{
    if (g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
        return rbGObject_marshal_string(g_variant_get_string(variant, NULL));

    if (g_variant_is_of_type(variant, G_VARIANT_TYPE_INT64))
        return LL2NUM(g_variant_get_int64(variant));

    if (g_variant_is_of_type(variant, G_VARIANT_TYPE_BOOLEAN))
        return (g_variant_get_boolean(variant) ? Qtrue : Qfalse);

    if (g_variant_is_of_type(variant, G_VARIANT_TYPE("ay"))) {
        gconstpointer elements;
        gsize n_elements;

        elements = g_variant_get_fixed_array(variant, &n_elements, sizeof(guint8));

        return rb_str_new(elements, n_elements);
    }

    if (g_variant_is_of_type(variant, G_VARIANT_TYPE_VARDICT)) {
        VALUE hash;
        GVariantIter iter;
        gchar * key;
        GVariant * raw_value;

        hash = rb_hash_new();

        g_variant_iter_init (&iter, variant);

        while (g_variant_iter_next(&iter, "{sv}", &key, &raw_value)) {
            rb_hash_aset(hash, ID2SYM(rb_intern(key)), rbGObject_marshal_variant(raw_value));
            g_variant_unref(raw_value);
            g_free (key);
        }
        return hash;
    }

    if (g_variant_is_of_type (variant, G_VARIANT_TYPE_ARRAY)) {
        GVariantIter iter;
        VALUE list;
        guint i;
        GVariant * child;

        g_variant_iter_init (&iter, variant);

        list = rb_ary_new_capa(g_variant_iter_n_children(&iter));

        for (i = 0; (child = g_variant_iter_next_value (&iter)) != NULL; i++) {
            rb_ary_store(list, i, rbGObject_marshal_variant(child));
            g_variant_unref (child);
        }
        return list;
    }
    return (Qnil);
}

gboolean rbGObject_unmarshal_certificate(const gchar *str, GTlsCertificate **certificate)
{
    GError * error = NULL;

    if (strchr(str, '\n') != NULL)
        *certificate = g_tls_certificate_new_from_pem(str, -1, &error);
    else
        *certificate = g_tls_certificate_new_from_file(str, &error);
    if (error != NULL)
        goto propagate_error;

    return TRUE;

propagate_error: {
        raise_rerror(NULL, error);

        return FALSE;
    }
}

VALUE rbGObject_marshal_dict(GHashTable *dict)
{
    VALUE result;
    GHashTableIter iter;
    const gchar * key;
    GVariant * raw_value;

    result = rb_hash_new();

    g_hash_table_iter_init(&iter, dict);
    while (g_hash_table_iter_next (&iter, (gpointer *) &key, (gpointer *) &raw_value)) {
        rb_hash_aset(result, ID2SYM(rb_intern(key)), rbGObject_marshal_variant(raw_value));
    }
    return result;
}

gboolean rbGObject_unmarshal_enum(const gchar *str, GType type, gpointer value)
{
    GEnumClass * enum_class;
    GEnumValue * enum_value;

    enum_class = g_type_class_ref(type);

    enum_value = g_enum_get_value_by_nick(enum_class, str);
    if (enum_value == NULL)
        goto invalid_value;

    *((gint *) value) = enum_value->value;

    g_type_class_unref(enum_class);

    return TRUE;

invalid_value: {
        return FALSE;
    }
}

static VALUE rbGObject_marshal_datetime(const gchar * iso8601_text)
{
    VALUE result;
    GDateTime * raw_dt, * dt;

    raw_dt = g_date_time_new_from_iso8601(iso8601_text, NULL);
    if (raw_dt == NULL)
        return (Qnil);

    dt = g_date_time_to_local (raw_dt);
    result = rb_funcall(rb_const_get(rb_cObject, rb_intern("DateTime")), rb_intern("new"), 7,
                        LL2NUM(g_date_time_get_year(dt)),
                        LL2NUM(g_date_time_get_month(dt)),
                        LL2NUM(g_date_time_get_day_of_month(dt)),
                        LL2NUM(g_date_time_get_hour(dt)),
                        LL2NUM(g_date_time_get_minute(dt)),
                        LL2NUM(g_date_time_get_second(dt)),
                        LL2NUM(g_date_time_get_microsecond(dt)));

    g_date_time_unref(dt);
    g_date_time_unref(raw_dt);

    return result;
}

VALUE rbApplication_marshal_parameters_dict(GHashTable * dict)
{
    VALUE result;
    GHashTableIter iter;
    const gchar * key;
    GVariant * raw_value;

    result = rb_hash_new();

    g_hash_table_iter_init(&iter, dict);

    while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&raw_value)) {
        VALUE value;

        if (strcmp(key, "started") == 0 && g_variant_is_of_type(raw_value, G_VARIANT_TYPE_STRING))
            value = rbGObject_marshal_datetime(g_variant_get_string (raw_value, NULL));
        else
            value = rbGObject_marshal_variant(raw_value);

        rb_hash_aset(result, ID2SYM(rb_intern(key)), value);
    }

    return result;
}

gboolean rbGObject_unmarshal_strv(VALUE value, gchar ***strv, gint *length)
{
    gint n, i;
    gchar ** elements;

    n = RARRAY_LEN(value);
    elements = g_new0(gchar *, n + 1);

    for (i = 0; i != n; i++) {
        VALUE element;

        element = RARRAY_AREF(value, i);
        elements[i] = g_strdup(StringValueCStr(element));
        if (elements[i] == NULL)
            goto invalid_element;
    }

    *strv = elements;
    *length = n;

    return TRUE;

invalid_element: {
        g_strfreev(elements);
        return FALSE;
    }
}

gboolean rbGObject_unmarshal_envp(VALUE hash, gchar ***envp, gint *length)
{
    gint n;
    gchar **elements;
    VALUE keys, name, value;
    gchar *raw_name = NULL;
    gchar *raw_value = NULL;

    keys = rb_funcall(hash, rb_intern("keys"), 0);
    n = RARRAY_LEN(hash);
    elements = g_new0 (gchar *, n + 1);

    for (int i = 0; i < n; i++) {
        name = RARRAY_AREF(keys, i);
        raw_name = g_strdup(StringValueCStr(name));
        value = rb_hash_aref(hash, name);
        raw_value = g_strdup(StringValueCStr(value));
        elements[i] = g_strconcat (raw_name, "=", raw_value, NULL);

        g_free(g_steal_pointer(&raw_value));
        g_free(g_steal_pointer(&raw_name));
    }

    *envp = elements;
    *length = n;

    return TRUE;
}

gboolean rbGObject_unmarshal_variant(VALUE value, GVariant ** variant)
{
    if (RB_TYPE_P(value, T_STRING)) {
        gchar *str;

        rbGObject_unmarshal_string(value, &str);

        *variant = g_variant_new_take_string(str);
    } else if (value == Qtrue || value == Qfalse) {
        *variant = g_variant_new_boolean(value == Qtrue);
    } else if (RB_TYPE_P(value, T_FIXNUM)) {
        long long l;

        l = NUM2LL(value);
        *variant = g_variant_new_int64(l);
    } else {
        goto unsupported_type;
    }
    return TRUE;

unsupported_type: {
        raise_argerror("unsupported type");
        goto propagate_error;
    }
propagate_error: {
        return FALSE;
    }
}

VALUE rbGObject_marshal_strv(gchar **strv, gint length)
{
    VALUE result;
    gint i;

    if (strv == NULL)
        return (Qnil);

    result = rb_ary_new_capa(length);
    for (i = 0; i != length; i++) {
        rb_ary_store(result, i, rbGObject_marshal_string(strv[i]));
    }

    return result;
}

VALUE rbGObject_marshal_envp(const gchar **envp, gint length)
{
    VALUE result;
    gint i;

    if (envp == NULL)
        return (Qnil);

    result = rb_hash_new();

    for (i = 0; i != length; i++) {
        gchar **tokens;

        tokens = g_strsplit(envp[i], "=", 2);

        if (g_strv_length(tokens) == 2) {
            const gchar * name;
            VALUE value;

            name = tokens[0];
            value = rbGObject_marshal_string(tokens[1]);

            rb_hash_aset(result, ID2SYM(rb_intern(name)), value);
        }

        g_strfreev(tokens);
    }

    return result;
}

VALUE rbGObject_marshal_bytes(GBytes * bytes)
{
    if (!bytes)	return (Qnil);
    gconstpointer data;
    gsize size;

    data = g_bytes_get_data (bytes, &size);
    return (rb_str_new((const char *)data, size));
}

static VALUE rbGObject_marshal_object(gpointer handle, GType type)
{
    VALUE object = Qnil;

    if (!handle)
        return (Qnil);
    g_object_ref(handle);
    if (type == FRIDA_TYPE_DEVICE_MANAGER)
        object = DeviceManager_from_FridaDeviceManager(handle);
    else if (type ==  FRIDA_TYPE_DEVICE)
        object = Device_from_FridaDevice(handle);
    else if (type ==  FRIDA_TYPE_APPLICATION)
        object = Application_from_FridaApplication(handle);
    else if (type ==  FRIDA_TYPE_PROCESS)
        object = Process_from_FridaProcess(handle);
    else if (type ==  FRIDA_TYPE_SPAWN)
        object = Spawn_from_FridaSpawn(handle);
    else if (type ==  FRIDA_TYPE_CHILD)
        object = Child_from_FridaChild(handle);
    else if (type ==  FRIDA_TYPE_BUS)
        object = Bus_from_FridaBus(handle);
    else if (type ==  FRIDA_TYPE_SESSION)
        object = Session_from_FridaSession(handle);
    else if (type ==  FRIDA_TYPE_SCRIPT)
        object = Script_from_FridaScript(handle);
    else if (type ==  FRIDA_TYPE_RELAY)
        object = Relay_from_FridaRelay(handle);
    else if (type ==  FRIDA_TYPE_PORTAL_MEMBERSHIP)
        object = PortalMembership_from_FridaPortalMembership(handle);
    else if (type ==  G_TYPE_IO_STREAM)
        object = IOStream_from_GIOStream(handle);
    else if (type == FRIDA_TYPE_CRASH)
        object = Crash_from_FridaCrash(handle);
    else if (type == FRIDA_TYPE_FILE_MONITOR)
        object = FileMonitor_from_FridaFileMonitor(handle);
    else if (type == FRIDA_TYPE_COMPILER)
        object = Compiler_from_FridaCompiler(handle);
    return (object);
}

static VALUE rbGObject_marshal_socket_address(GSocketAddress * address)
{
    VALUE result = Qnil;

    if (G_IS_INET_SOCKET_ADDRESS (address)) {
        GInetSocketAddress * sa;
        GInetAddress * ia;
        gchar * host;
        guint16 port;

        sa = G_INET_SOCKET_ADDRESS (address);
        ia = g_inet_socket_address_get_address (sa);

        host = g_inet_address_to_string (ia);
        port = g_inet_socket_address_get_port (sa);

        if (g_socket_address_get_family (address) == G_SOCKET_FAMILY_IPV4)
            result = rb_sprintf("(%s, %d)", host, port);
        else
            result = rb_sprintf("(%s, %d, %d, %d)", host, port, g_inet_socket_address_get_flowinfo(sa), g_inet_socket_address_get_scope_id(sa));

        g_free (host);
    } else if (G_IS_UNIX_SOCKET_ADDRESS (address)) {
        GUnixSocketAddress * sa = G_UNIX_SOCKET_ADDRESS (address);

        switch (g_unix_socket_address_get_address_type (sa)) {
        case G_UNIX_SOCKET_ADDRESS_ANONYMOUS: {
            result = rb_str_new_cstr ("");
            break;
        }
        case G_UNIX_SOCKET_ADDRESS_PATH: {
            gchar * path = g_filename_to_utf8 (g_unix_socket_address_get_path (sa), -1, NULL, NULL, NULL);
            result = rb_str_new_cstr (path);
            g_free (path);
            break;
        }
        case G_UNIX_SOCKET_ADDRESS_ABSTRACT:
        case G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED: {
            result = rb_str_new (g_unix_socket_address_get_path (sa), g_unix_socket_address_get_path_len (sa));
            break;
        }
        default: {
            result = Qnil;
            break;
        }
        }
    }
    return result;
}

VALUE rbGObject_marshal_value(const GValue *value)
{
    GType type;

    type = G_VALUE_TYPE(value);
    switch (type) {
    case G_TYPE_BOOLEAN:
        return (g_value_get_boolean(value) ? Qtrue : Qfalse);

    case G_TYPE_INT:
        return LL2NUM(g_value_get_int(value));

    case G_TYPE_UINT:
        return UINT2NUM(g_value_get_uint(value));

    case G_TYPE_FLOAT:
        return DBL2NUM(g_value_get_float(value));

    case G_TYPE_DOUBLE:
        return DBL2NUM(g_value_get_double(value));

    case G_TYPE_STRING:
        return rbGObject_marshal_string(g_value_get_string(value));

    case G_TYPE_VARIANT:
        return rbGObject_marshal_variant(g_value_get_variant(value));

    default:
        if (G_TYPE_IS_ENUM(type))
            return rbGObject_marshal_enum(g_value_get_enum(value), type);

        if (type == G_TYPE_BYTES) {
            return rbGObject_marshal_bytes(g_value_get_boxed(value));
        }

        if (G_TYPE_IS_OBJECT(type)) {
            if (G_IS_SOCKET_ADDRESS(g_value_get_object(value)))
                return (rbGObject_marshal_socket_address(g_value_get_object(value)));
            return rbGObject_marshal_object(g_value_get_object(value), type);
        }
        goto unsupported_type;
    }

    g_assert_not_reached();

unsupported_type: {
        raise_rerror("unsupported type", NULL);
        return (Qnil);
    }
}

VALUE *rbGObjectSignalClosure_marshal_params(const GValue *params, guint params_length)
{
    VALUE *args;

    args = malloc(sizeof(VALUE) * params_length);

    for (uint i = 0; i < params_length; i++) {
        VALUE arg;

        arg = rbGObject_marshal_value(&params[i]);
        args[i] = arg;
    }
    return args;
}
