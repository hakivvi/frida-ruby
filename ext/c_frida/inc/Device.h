#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cDevice;

typedef struct {
    FridaDevice	*device_handle;
    char		*address;
} open_channel_proxy_args;

typedef struct {
    FridaDevice	*device_handle;
    FridaFrontmostQueryOptions *options;
} get_frontmost_application_proxy_args;

typedef struct {
    FridaDevice	*device_handle;
    FridaProcessQueryOptions *options;
} enumerate_processes_proxy_args;

typedef struct {
    FridaDevice	*device_handle;
    FridaApplicationQueryOptions *options;
} enumerate_applications_proxy_args;

typedef struct {
    FridaDevice	*device_handle;
    char		*program;
    FridaSpawnOptions *options;
} spawn_sync_proxy_args;

typedef struct {
    FridaDevice *device_handle;
    guint		pid;
} resume_sync_proxy_args;

typedef resume_sync_proxy_args kill_sync_proxy_args;

typedef struct {
    FridaDevice *device_handle;
    guint		pid;
    GBytes 		*data;
} input_sync_proxy_args;

typedef struct {
    FridaDevice *device_handle;
    guint		pid;
    char		*path;
    char		*entrypoint;
    char		*data;
} inject_library_file_sync_proxy_args;

typedef struct {
    FridaDevice *device_handle;
    guint		pid;
    GBytes		*blob;
    char		*entrypoint;
    char		*data;
} inject_library_blob_sync_proxy_args;

typedef struct {
    FridaDevice *device_handle;
    guint		pid;
    FridaSessionOptions	*options;
} attach_sync_proxy_args;

void	define_Device();
VALUE	Device_from_FridaDevice(FridaDevice *device);
int		array_all_type(VALUE arr, VALUE rtype);
