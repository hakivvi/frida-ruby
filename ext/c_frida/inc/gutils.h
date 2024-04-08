#pragma once

#include "c_frida.h"

VALUE rbGObject_marshal_string(const gchar * str);
VALUE rbGObject_marshal_enum(gint value, GType type);
VALUE rbGObject_marshal_variant(GVariant * variant);
gboolean rbGObject_unmarshal_certificate(const gchar *str, GTlsCertificate **certificate);
gboolean rbGObject_unmarshal_enum(const gchar *str, GType type, gpointer value);
VALUE	rbGObject_marshal_dict(GHashTable *dict);
VALUE rbApplication_marshal_parameters_dict(GHashTable * dict);
#define rbProcess_marshal_parameters_dict rbApplication_marshal_parameters_dict
gboolean rbGObject_unmarshal_strv(VALUE value, gchar ***strv, gint *length);
gboolean rbGObject_unmarshal_envp(VALUE hash, gchar ***envp, gint *length);
gboolean rbGObject_unmarshal_string(VALUE value, gchar **str);
gboolean rbGObject_unmarshal_variant(VALUE value, GVariant ** variant);
VALUE rbGObject_marshal_strv(gchar **strv, gint length);
VALUE rbGObject_marshal_envp(const gchar **envp, gint length);
VALUE rbGObject_marshal_value(const GValue *value);
VALUE *rbGObjectSignalClosure_marshal_params(const GValue *params, guint params_length);
VALUE rbGObject_marshal_bytes(GBytes * bytes);
