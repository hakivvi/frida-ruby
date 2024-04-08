#pragma once

#include "c_frida.h"

typedef struct {
    FridaPortalService	*handle;
    guint               connection_id;
} kick_proxy_args;

typedef struct {
    FridaPortalService	*handle;
    guint               connection_id;
    char                *message;
    GBytes              *data;
} _post_proxy_args;

typedef struct {
    FridaPortalService	*handle;
    char                *tag;
    char                *message;
    GBytes              *data;
} narrowcast_proxy_args;

typedef struct {
    FridaPortalService	*handle;
    char                *message;
    GBytes              *data;
} broadcast_proxy_args;

typedef struct {
    FridaPortalService	*handle;
    guint               connection_id;
    gint                *tags_count;
} enumerate_tags_proxy_args;

typedef struct {
    FridaPortalService	*handle;
    guint               connection_id;
    char                *tag;
} tag_proxy_args;

typedef tag_proxy_args untag_proxy_args;

extern VALUE mCFrida;
extern VALUE cPortalService;

void	define_PortalService();
