#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cSession;

typedef struct {
    FridaSession *handle;
    char		*source;
    FridaScriptOptions *options;
} create_script_sync_proxy_args;

typedef create_script_sync_proxy_args compile_script_sync_proxy_args;

typedef struct {
    FridaSession *handle;
    char		*embed_script;
    FridaSnapshotOptions *options;
} snapshot_script_sync_proxy_args;

typedef struct {
    FridaSession *handle;
    GBytes	*bytes;
    FridaScriptOptions *options;
} create_script_from_bytes_sync_proxy_args;

typedef struct {
    FridaSession *handle;
    FridaPeerOptions *options;
} setup_peer_connection_sync_proxy_args;

typedef struct {
    FridaSession		*handle;
    char				*address;
    FridaPortalOptions	*options;
} join_portal_sync_proxy_args;

void	define_Session();
VALUE	Session_from_FridaSession(FridaSession *stream);
