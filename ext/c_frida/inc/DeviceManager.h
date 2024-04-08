#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cDeviceManager;

typedef struct {
    FridaDeviceManager	*device_manager;
    char				*address;
    FridaRemoteDeviceOptions *options;
} add_remote_device_proxy_args;

typedef struct {
    FridaDeviceManager	*device_manager;
    char				*address;
} remove_remote_devices_proxy_args;

void	define_DeviceManager();
VALUE	DeviceManager_from_FridaDeviceManager(FridaDeviceManager *handle);
